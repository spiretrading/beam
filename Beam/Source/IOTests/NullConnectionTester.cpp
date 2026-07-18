module;
#include "Prelude.hpp"
#include <doctest/doctest.h>

module Beam;

using namespace Beam;

TEST_SUITE("NullConnection") {
  TEST_CASE("close") {
    auto connection = NullConnection();
    REQUIRE_NOTHROW(connection.close());
  }
}
