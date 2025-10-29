#include <string>
#include <doctest/doctest.h>
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;

TEST_SUITE("NullSlot") {
  TEST_CASE("no_args") {
    auto slot = NullSlot();
    REQUIRE_NOTHROW(slot());
  }

  TEST_CASE("with_args") {
    auto slot = NullSlot();
    auto text = std::string("hello");
    REQUIRE_NOTHROW(slot(1, 2.5, 'c', text));
    REQUIRE(text == "hello");
    REQUIRE_NOTHROW(slot(std::string("temp")));
  }

  TEST_CASE("rvalue_and_lvalue_forwarding") {
    auto slot = NullSlot();
    auto number = 10;
    REQUIRE_NOTHROW(slot(number, std::move(number)));
  }
}
