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
  using TestDataStore = SqlDataStore<Sqlite3::Connection,
    BasicQuery<std::string>, Row<SequencedIndexedEntry>, SqlTranslator>;

  bool Entry::operator ==(const Entry& rhs) const {
    return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
  }

  const auto PATH = "file:memdb?mode=memory&cache=shared";

  auto BuildRow() {
    return Row<SequencedIndexedEntry>().
      add_column("name",
        [] (auto& row) {
          return row->GetIndex();
        },
        [] (auto& row, auto&& value) {
          row->GetIndex() = std::forward<decltype(value)>(value);
        })/*.
      add_column("value",
        [] (auto& row) -> decltype(auto) {
          return (*row)->m_value;
        })*/;
  }

  auto StoreValue(TestDataStore& dataStore, std::string index, int value,
      const ptime& timestamp, const Queries::Sequence& sequence) {
    auto entry = MakeSequencedValue(MakeIndexedValue(Entry{value, timestamp},
      index), sequence);
    dataStore.Store(entry);
    return entry;
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
  auto reader = connectionPool.Acquire();
  std::vector<SequencedIndexedEntry> rows;
  reader->execute(select(BuildRow(), "test", std::back_inserter(rows)));
  CPPUNIT_ASSERT(rows.size() == 1);
}
