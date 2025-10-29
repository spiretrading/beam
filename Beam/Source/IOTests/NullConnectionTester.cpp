#include <doctest/doctest.h>
#include "Beam/IO/NullConnection.hpp"

using namespace Beam;

TEST_SUITE("NullConnection") {
  TEST_CASE("close") {
    auto connection = NullConnection();
    REQUIRE_NOTHROW(connection.close());
  }
}
