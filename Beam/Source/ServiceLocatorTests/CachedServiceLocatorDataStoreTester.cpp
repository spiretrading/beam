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
      return CachedServiceLocatorDataStore<LocalServiceLocatorDataStore>(
        init());
    }
  };
}

TEST_SUITE("CachedServiceLocatorDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(ServiceLocatorDataStoreTestSuite, Builder);
}
