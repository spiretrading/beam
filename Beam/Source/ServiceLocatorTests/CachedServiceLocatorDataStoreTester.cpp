#include <doctest/doctest.h>
#include "Beam/ServiceLocator/CachedServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorDataStoreTestSuite.hpp"

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
