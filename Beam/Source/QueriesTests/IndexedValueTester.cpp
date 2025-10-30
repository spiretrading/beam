#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/IndexedValue.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("IndexedValue") {
  TEST_CASE("default_constructor") {
    auto int_value = IndexedValue<int, std::string>();
    REQUIRE(int_value.get_index().empty());
    REQUIRE(int_value.get_value() == 0);
    auto string_value = IndexedValue<std::string, std::string>();
    REQUIRE(string_value.get_index().empty());
    REQUIRE(string_value.get_value().empty());
  }

  TEST_CASE("value_and_sequence_constructor") {
    auto value =
      IndexedValue<std::string, std::string>("hello world", "goodbye sky");
    REQUIRE(value.get_value() == "hello world");
    REQUIRE(value.get_index() == "goodbye sky");
  }

  TEST_CASE("dereference") {
    auto int_value = IndexedValue<int, std::string>(123, "index");
    REQUIRE(*int_value == 123);
    auto string_value =
      IndexedValue<std::string, std::string>("hello world", "index");
    REQUIRE(*string_value == "hello world");
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
    REQUIRE(value.get_value() == 321);
    REQUIRE(value.get_index() == "hello world");
  }

  TEST_CASE("stream") {
    REQUIRE(
      to_string(IndexedValue(123, std::string("index"))) == "(index 123)");
    REQUIRE(to_string(
      IndexedValue(std::string("hello world"), std::string("id"))) ==
        "(id hello world)");
    REQUIRE(to_string(IndexedValue<int, std::string>()) == "( 0)");
    test_round_trip_shuttle(IndexedValue(123, std::string("index")));
  }
}
