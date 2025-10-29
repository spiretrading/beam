#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/NamedChannelIdentifier.hpp"

using namespace Beam;

TEST_SUITE("NamedChannelIdentifier") {
  TEST_CASE("default_constructor") {
    auto identifier = NamedChannelIdentifier();
    auto ss = std::stringstream();
    ss << identifier;
    REQUIRE(ss.str().empty());
  }

  TEST_CASE("constructor") {
    auto identifier = NamedChannelIdentifier("test_name");
    auto ss = std::stringstream();
    ss << identifier;
    REQUIRE(ss.str() == "test_name");
  }
}
