#include <doctest/doctest.h>
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidServiceTests/UidDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return LocalUidDataStore();
    }
  };
}

TEST_SUITE("LocalUidDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(UidDataStoreTestSuite, Builder);
}
