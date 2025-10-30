#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

namespace {
  struct CustomStreamable {
    int m_value;

    explicit CustomStreamable(int value)
      : m_value(value) {}
  };

  std::ostream& operator <<(std::ostream& out, const CustomStreamable& obj) {
    return out << "CustomStreamable(" << obj.m_value << ')';
  }
}

TEST_SUITE("ToString") {
  TEST_CASE("built_in_types") {
    REQUIRE(to_string(42) == "42");
    REQUIRE(to_string(-123) == "-123");
    REQUIRE(to_string(0.25) == "0.25");
    REQUIRE(to_string(true) == "1");
    REQUIRE(to_string(false) == "0");
  }

  TEST_CASE("string_types") {
    REQUIRE(to_string(std::string("hello")) == "hello");
    REQUIRE(to_string("world") == "world");
  }

  TEST_CASE("custom_streamable_type") {
    auto obj = CustomStreamable(99);
    REQUIRE(to_string(obj) == "CustomStreamable(99)");
  }

  TEST_CASE("pointer") {
    auto value = 42;
    auto ptr = &value;
    auto result = to_string(ptr);
    REQUIRE(!result.empty());
  }

  TEST_CASE("character") {
    REQUIRE(to_string('A') == "A");
    REQUIRE(to_string('z') == "z");
  }

  TEST_CASE("floating_point_precision") {
    auto result = to_string(1.23456789);
    REQUIRE(result.find("1.234") != std::string::npos);
  }
}
