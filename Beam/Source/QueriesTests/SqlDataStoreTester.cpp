#include <vector>
#include <doctest/doctest.h>
#include <Viper/Sqlite3/Sqlite3.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SqlDataStore.hpp"
#include "Beam/QueriesTests/TestEntry.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Viper;

namespace {
  using DataStore = SqlDataStore<
    Sqlite3::Connection, Row<TestEntry>, Row<std::string>, SqlTranslator>;

  const auto PATH = "file:memdb?mode=memory&cache=shared";

  auto make_value_row() {
    return Row<TestEntry>().add_column("value", &TestEntry::m_value);
  }

  auto make_index_row() {
    return Row<std::string>().add_column("name");
  }
}

TEST_SUITE("SqlDataStore") {
  TEST_CASE("store_and_load") {
    auto reader_pool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto writer_pool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto data_store = DataStore("test", make_value_row(), make_index_row(),
      Ref(reader_pool), Ref(writer_pool));
    auto sequence = Beam::Sequence(5);
    auto entry_a = store(data_store, "hello", 100,
      time_from_string("2022-04-06 04:15:22:01"), sequence);
    test_query(data_store, "hello", Range::TOTAL, SnapshotLimit::from_tail(1),
      {entry_a});
  }

  TEST_CASE("embedded_index") {
    auto make_embedded_index_row = [] {
      return Row<int>().add_column("value");
    };
    using EmbeddedDataStore = SqlDataStore<
      Sqlite3::Connection, Row<TestEntry>, Row<int>, SqlTranslator>;
    auto reader_pool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto writer_pool = DatabaseConnectionPool<Sqlite3::Connection>(1,
      [] {
        auto connection = std::make_unique<Sqlite3::Connection>(PATH);
        connection->open();
        return connection;
      });
    auto data_store = EmbeddedDataStore("test", make_value_row(),
      make_embedded_index_row(), Ref(reader_pool), Ref(writer_pool));
  }
}
