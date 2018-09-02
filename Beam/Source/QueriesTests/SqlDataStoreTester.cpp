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

  struct IndexBuilder {
    auto operator ()(const std::string& value) const {
      return sym("name") == value;
    }
  };

  template<typename T>
  auto FIndexBuilder(const Row<T>& row, const T& value) {
    std::optional<Viper::Expression> e;
    std::string column;
    for(auto i = std::size_t(0); i != row.get_columns().size(); ++i) {
      row.append_value(value, i, column);
      auto term = Viper::sym(row.get_columns()[i].m_name) == column;
      if(e.has_value()) {
        e = e && term;
      } else {
        e.emplace(std::move(term));
      }
      column.clear();
    }
    if(e.has_value()) {
      return *e;
    }
    return {};
  }

  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry = SequencedValue<IndexedValue<Entry,
    std::string>>;
  using TestDataStore = SqlDataStore<Sqlite3::Connection, Row<Entry>,
    Row<std::string>, IndexBuilder, SqlTranslator>;

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  const auto PATH = "file:memdb?mode=memory&cache=shared";

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

  auto BuildValueRow() {
    return Row<Entry>().
      add_column("value", &Entry::m_value);
  }

  auto BuildIndexRow() {
    return Row<std::string>().add_column("name");
  }
}

void SqlDataStoreTester::TestStoreAndLoad() {
  auto connectionPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto c = std::make_unique<Sqlite3::Connection>(PATH);
  c->open();
  connectionPool.Add(std::move(c));
  auto writerConnection = Sync<Sqlite3::Connection, Mutex>(PATH);
  auto threadPool = ThreadPool();
  auto dataStore = TestDataStore("test", BuildValueRow(), BuildIndexRow(),
    Ref(connectionPool), Ref(writerConnection), Ref(threadPool));
  dataStore.Open();
  auto timeClient = IncrementalTimeClient();
  auto sequence = Queries::Sequence(5);
  auto entryA = StoreValue(dataStore, "hello", 100, timeClient.GetTime(),
    sequence);
  TestQuery(dataStore, "hello", Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryA});
}
