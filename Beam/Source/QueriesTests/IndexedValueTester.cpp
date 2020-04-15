#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/IndexedValue.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("IndexedValue") {
  TEST_CASE("default_constructor") {
    auto intValue = IndexedValue<int, std::string>();
    REQUIRE(intValue.GetIndex().empty());
    REQUIRE(intValue.GetValue() == 0);
    auto stringValue = IndexedValue<std::string, std::string>();
    REQUIRE(stringValue.GetIndex().empty());
    REQUIRE(stringValue.GetValue().empty());
  }

  TEST_CASE("value_and_sequence_constructor") {
    auto value = IndexedValue<std::string, std::string>("hello world",
      "goodbye sky");
    REQUIRE(value.GetValue() == "hello world");
    REQUIRE(value.GetIndex() == "goodbye sky");
  }

  TEST_CASE("dereference") {
    auto intValue = IndexedValue<int, std::string>(123, "index");
    REQUIRE(*intValue == 123);
    auto stringValue = IndexedValue<std::string, std::string>(
      "hello world", "index");
    REQUIRE(*stringValue == "hello world");
  }

  TEST_CASE("equals_operator") {
    using TestIndexedValue = IndexedValue<int, std::string>;
    REQUIRE(TestIndexedValue(123, "hello world") ==
      TestIndexedValue(123, "hello world"));
    REQUIRE(!(TestIndexedValue(123, "hello world") ==
      TestIndexedValue(321, "hello world")));
    REQUIRE(!(TestIndexedValue(123, "goodbye sky") ==
      TestIndexedValue(123, "hello world")));
    REQUIRE(!(TestIndexedValue(123, "goodbye sky") ==
      TestIndexedValue(321, "hello world")));
  }

  TEST_CASE("not_equals_operator") {
    using TestIndexedValue = IndexedValue<int, std::string>;
    REQUIRE(!(TestIndexedValue(123, "hello world") !=
      TestIndexedValue(123, "hello world")));
    REQUIRE(TestIndexedValue(123, "hello world") !=
      TestIndexedValue(321, "hello world"));
    REQUIRE(TestIndexedValue(123, "goodbye sky") !=
      TestIndexedValue(123, "hello world"));
    REQUIRE(TestIndexedValue(123, "goodbye sky") !=
      TestIndexedValue(321, "hello world"));
  }

  TEST_CASE("make_indexed_value") {
    auto value = IndexedValue(321, std::string("hello world"));
    REQUIRE(value.GetValue() == 321);
    REQUIRE(value.GetIndex() == "hello world");
  }
}
