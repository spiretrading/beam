#include <doctest/doctest.h>
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
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
