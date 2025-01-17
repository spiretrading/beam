#include <vector>
#include <doctest/doctest.h>
#include <Viper/Sqlite3/Sqlite3.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SqlDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"
#include "Beam/TimeService/IncrementalTimeClient.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace Beam::TimeService;
using namespace Viper;

namespace {
  using DataStore = SqlDataStore<Sqlite3::Connection, Row<TestEntry>,
    Row<std::string>, SqlTranslator>;

  const auto PATH = "file:memdb?mode=memory&cache=shared";

  auto MakeValueRow() {
    return Row<TestEntry>().add_column("value", &TestEntry::m_value);
  }

  auto MakeIndexRow() {
    return Row<std::string>().add_column("name");
  }
}

TEST_SUITE("SqlDataStore") {
  TEST_CASE("store_and_load") {
    auto readerPool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto writerPool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto dataStore = DataStore("test", MakeValueRow(), MakeIndexRow(),
      Ref(readerPool), Ref(writerPool));
    auto timeClient = IncrementalTimeClient();
    auto sequence = Queries::Sequence(5);
    auto entryA = StoreValue(dataStore, "hello", 100, timeClient.GetTime(),
      sequence);
    TestQuery(dataStore, "hello", Queries::Range::Total(),
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1), {entryA});
  }

  TEST_CASE("embedded_index") {
    auto MakeEmbeddedIndexRow = [] {
      return Row<int>().add_column("value");
    };
    using EmbeddedDataStore = SqlDataStore<Sqlite3::Connection, Row<TestEntry>,
      Row<int>, SqlTranslator>;
    auto readerPool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto writerPool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto dataStore = EmbeddedDataStore("test", MakeValueRow(),
      MakeEmbeddedIndexRow(), Ref(readerPool), Ref(writerPool));
  }
}
