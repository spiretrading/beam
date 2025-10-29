#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Beam/Sql/SqlConnection.hpp"
#include "Beam/ServiceLocator/SqlServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlServiceLocatorDataStore =
    SqlServiceLocatorDataStore<SqlConnection<Sqlite3::Connection>>;

  struct Builder {
    auto operator ()() const {
      return TestSqlServiceLocatorDataStore(
        std::make_unique<SqlConnection<Sqlite3::Connection>>(
          "file::memory:?cache=shared"));
    }
  };
}

TEST_SUITE("SqlServiceLocatorDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(ServiceLocatorDataStoreTestSuite, Builder);
}
