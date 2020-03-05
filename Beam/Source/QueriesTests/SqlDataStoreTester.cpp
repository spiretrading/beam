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

    bool operator ==(const Entry& rhs) const {
      return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
    }
  };

  using SequencedEntry = SequencedValue<Entry>;
  using SequencedIndexedEntry = SequencedValue<IndexedValue<Entry,
    std::string>>;
  using DataStore = SqlDataStore<Sqlite3::Connection, Row<Entry>,
    Row<std::string>, SqlTranslator>;

  const auto PATH = "file:memdb?mode=memory&cache=shared";

  auto StoreValue(DataStore& dataStore, std::string index, int value,
      const ptime& timestamp, const Queries::Sequence& sequence) {
    auto entry = SequencedValue(IndexedValue(Entry{value, timestamp}, index),
      sequence);
    dataStore.Store(entry);
    return entry;
  }

  void TestQuery(DataStore& dataStore, std::string index,
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
    return Row<Entry>().add_column("value", &Entry::m_value);
  }

  auto BuildIndexRow() {
    return Row<std::string>().add_column("name");
  }
}

void SqlDataStoreTester::TestStoreAndLoad() {
  auto readerPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto writerPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto connection = std::make_unique<Sqlite3::Connection>(PATH);
  connection->open();
  readerPool.Add(std::move(connection));
  connection = std::make_unique<Sqlite3::Connection>(PATH);
  connection->open();
  writerPool.Add(std::move(connection));
  auto threadPool = ThreadPool();
  auto dataStore = DataStore("test", BuildValueRow(), BuildIndexRow(),
    Ref(readerPool), Ref(writerPool), Ref(threadPool));
  dataStore.Open();
  auto timeClient = IncrementalTimeClient();
  auto sequence = Queries::Sequence(5);
  auto entryA = StoreValue(dataStore, "hello", 100, timeClient.GetTime(),
    sequence);
  TestQuery(dataStore, "hello", Queries::Range::Total(),
    SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryA});
}

void SqlDataStoreTester::TestEmbeddedIndex() {
  auto BuildEmbeddedValueRow = [] {
    return Row<Entry>().add_column("value", &Entry::m_value);
  };
  auto BuildEmbeddedIndexRow = [] {
    return Row<int>().add_column("value");
  };
  using EmbeddedDataStore = SqlDataStore<Sqlite3::Connection, Row<Entry>,
    Row<int>, SqlTranslator>;
  auto connectionPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto readerPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto writerPool = DatabaseConnectionPool<Sqlite3::Connection>();
  auto connection = std::make_unique<Sqlite3::Connection>(PATH);
  connection->open();
  readerPool.Add(std::move(connection));
  connection = std::make_unique<Sqlite3::Connection>(PATH);
  connection->open();
  writerPool.Add(std::move(connection));
  auto threadPool = ThreadPool();
  auto dataStore = EmbeddedDataStore("test", BuildEmbeddedValueRow(),
    BuildEmbeddedIndexRow(), Ref(readerPool), Ref(writerPool), Ref(threadPool));
  dataStore.Open();
}
