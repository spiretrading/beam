module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include "Beam/ServiceLocatorTests/ServiceLocatorDataStoreTestSuite.hpp"

module Beam;

using namespace Beam;
using namespace Beam::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return LocalServiceLocatorDataStore();
    }
  };
}

TEST_SUITE("LocalServiceLocatorDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(ServiceLocatorDataStoreTestSuite, Builder);
}
