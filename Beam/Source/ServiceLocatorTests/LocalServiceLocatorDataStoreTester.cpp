module;
#include "Prelude.hpp"
#include <doctest/doctest.h>

module Beam;

#include "Beam/ServiceLocatorTests/ServiceLocatorDataStoreTestSuite.hpp"

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
