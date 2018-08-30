#include "Beam/QueriesTests/SqlDataStoreTester.hpp"
#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Viper/Sqlite3/Sqlite3.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SqlDataStore.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::Threading;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::posix_time;
using namespace Viper;

namespace {
  struct Entry {
    int m_value;
    ptime m_timestamp;

    bool operator ==(const Entry& rhs) const;
  };

  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry = SequencedValue<IndexedValue<Entry,
    std::string>>;

  struct EntryIndexBuilder {
    auto operator ()(const std::string& index) const {
      return sym("name") == index;
    }
  };

  using TestDataStore = SqlDataStore<Sqlite3::Connection,
    BasicQuery<std::string>, Row<SequencedIndexedEntry>, EntryIndexBuilder,
    SqlTranslator>;

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  const auto PATH = "file:memdb?mode=memory&cache=shared";

  template<typename G, typename S>
  struct TimestampAccessor {
    G m_getter;
    S m_setter;

    TimestampAccessor(G getter, S setter)
        : m_getter(std::move(getter)),
          m_setter(std::move(setter)) {}

    template<typename T>
    auto operator ()(T& row) const {
      return MySql::ToMySqlTimestamp(m_getter(row));
    }

    template<typename T, typename V>
    void operator ()(T& row, V value) const {
      m_setter(row, MySql::FromMySqlTimestamp(std::forward<V>(value)));
    }
  };

  template<typename F>
  auto MakeTimestampAccessor(F&& accessor) {
    auto getter = std::forward<F>(accessor);
    auto setter =
      [=] (auto& row, auto&& value) {
        getter(row) = std::forward<decltype(value)>(value);
      };
    return TimestampAccessor<decltype(getter), decltype(setter)>(
      std::move(getter), std::move(setter));
  }

  template<typename G, typename S>
  struct SequenceAccessor {
    G m_getter;
    S m_setter;

    SequenceAccessor(G getter, S setter)
        : m_getter(std::move(getter)),
          m_setter(std::move(setter)) {}

    template<typename T>
    auto operator ()(T& row) const {
      return m_getter(row).GetOrdinal();
    }

    template<typename T, typename V>
    void operator ()(T& row, V value) const {
      m_setter(row, Queries::Sequence(value));
    }
  };

  template<typename F>
  auto MakeSequenceAccessor(F&& accessor) {
    auto getter = std::forward<F>(accessor);
    auto setter =
      [=] (auto& row, auto&& value) {
        getter(row) = Queries::Sequence(std::forward<decltype(value)>(value));
      };
    return SequenceAccessor<decltype(getter), decltype(setter)>(
      std::move(getter), std::move(setter));
  }

  auto BuildRow() {
    return Row<SequencedIndexedEntry>().
      add_column("name",
        [] (auto& row) -> auto& {
          return row->GetIndex();
        }).
      add_column("value",
        [] (auto& row) -> auto& {
          return (*row)->m_value;
        }).
      add_column("timestamp",
        MakeTimestampAccessor(
          [] (auto& row) -> auto& {
            return (*row)->m_timestamp;
          })).
      add_column("query_sequence",
        MakeSequenceAccessor(
          [] (auto& row) -> decltype(auto) {
            return row.GetSequence();
          }));
  }

  auto StoreValue(TestDataStore& dataStore, std::string index, int value,
      const ptime& timestamp, const Queries::Sequence& sequence) {
    auto entry = MakeSequencedValue(MakeIndexedValue(Entry{value, timestamp},
      index), sequence);
    dataStore.Store(entry);
    return entry;
  }

  void TestQuery(TestDataStore& dataStore, std::string index,
      const Queries::Range& range, const SnapshotLimit& limit,
      const std::vector<SequencedEntry>& expectedResult) {
    BasicQuery<std::string> query;
    query.SetIndex(index);
    query.SetRange(range);
    query.SetSnapshotLimit(limit);
    auto queryResult = dataStore.Load(query);
    CPPUNIT_ASSERT(expectedResult == queryResult);
  }
}

void SqlDataStoreTester::TestStoreAndLoad() {
  auto connectionPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto c = std::make_unique<Sqlite3::Connection>(PATH);
  c->open();
  connectionPool.Add(std::move(c));
  auto writerConnection = Sync<Sqlite3::Connection, Mutex>(PATH);
  writerConnection.With(
    [&] (auto& connection) {
      connection.open();
      connection.execute(create_if_not_exists(BuildRow(), "test"));
    });
  auto threadPool = ThreadPool();
  auto dataStore = TestDataStore(BuildRow(), "test", Ref(connectionPool),
    Ref(writerConnection), Ref(threadPool));
  auto timeClient = IncrementalTimeClient();
  auto sequence = Queries::Sequence(5);
  auto entryA = StoreValue(dataStore, "hello", 100, timeClient.GetTime(),
    sequence);
  TestQuery(dataStore, "hello", Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryA});
}
