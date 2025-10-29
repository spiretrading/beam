#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Beam/Sql/SqlConnection.hpp"
#include "Beam/UidService/SqlUidDataStore.hpp"
#include "Beam/UidServiceTests/UidDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlUidDataStore =
    SqlUidDataStore<SqlConnection<Sqlite3::Connection>>;

  struct Builder {
    auto operator ()() const {
      return TestSqlUidDataStore(
        std::make_unique<SqlConnection<Sqlite3::Connection>>(
          "file::memory:?cache=shared"));
    }
  };
}

TEST_SUITE("SqlUidDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(UidDataStoreTestSuite, Builder);
}
