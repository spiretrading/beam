module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include "Beam/UidServiceTests/UidDataStoreTestSuite.hpp"

module Beam;

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
