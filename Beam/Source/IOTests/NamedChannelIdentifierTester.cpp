#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("NamedChannelIdentifier") {
  TEST_CASE("default_constructor") {
    auto identifier = NamedChannelIdentifier();
    REQUIRE(to_string(identifier).empty());
  }

  TEST_CASE("constructor") {
    auto identifier = NamedChannelIdentifier("test_name");
    REQUIRE(to_string(identifier) == "test_name");
  }
}
