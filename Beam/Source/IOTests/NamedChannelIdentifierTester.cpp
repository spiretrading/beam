module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>

module Beam;

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
