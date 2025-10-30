#include <doctest/doctest.h>
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/QueriesTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("SequencedValue") {
  TEST_CASE("default_constructor") {
    auto int_value = SequencedValue<int>();
    REQUIRE(int_value.get_sequence() == Sequence());
    REQUIRE(int_value.get_value() == 0);
    auto string_value = SequencedValue<std::string>();
    REQUIRE(string_value.get_sequence() == Sequence());
    REQUIRE(string_value.get_value() == "");
  }

  TEST_CASE("value_and_sequence_constructor") {
    auto value = SequencedValue(std::string("hello world"), Sequence(1));
    REQUIRE(value.get_value() == "hello world");
    REQUIRE(value.get_sequence() == Sequence(1));
  }

  TEST_CASE("dereference") {
    auto int_value = SequencedValue(123, Sequence(1));
    REQUIRE(*int_value == 123);
    auto string_value = SequencedValue(std::string("hello world"), Sequence(1));
    REQUIRE(*string_value == "hello world");
  }

  TEST_CASE("equals_operator") {
    REQUIRE(SequencedValue(std::string("hello world"), Sequence(1)) ==
      SequencedValue(std::string("hello world"), Sequence(1)));
    REQUIRE(!(SequencedValue(std::string("hello world"), Sequence(1)) ==
      SequencedValue(std::string("hello world"), Sequence(2))));
    REQUIRE(!(SequencedValue(std::string("goodbye sky"), Sequence(1)) ==
      SequencedValue(std::string("hello world"), Sequence(1))));
    REQUIRE(!(SequencedValue(std::string("goodbye sky"), Sequence(1)) ==
      SequencedValue(std::string("hello world"), Sequence(2))));
  }

  TEST_CASE("not_equals_operator") {
    REQUIRE(!(SequencedValue(std::string("hello world"), Sequence(1)) !=
      SequencedValue(std::string("hello world"), Sequence(1))));
    REQUIRE(SequencedValue(std::string("hello world"), Sequence(2)) !=
      SequencedValue(std::string("hello world"), Sequence(1)));
    REQUIRE(SequencedValue(std::string("goodbye sky"), Sequence(2)) !=
      SequencedValue(std::string("hello world"), Sequence(2)));
    REQUIRE(SequencedValue(std::string("goodbye sky"), Sequence(1)) !=
      SequencedValue(std::string("hello world"), Sequence(2)));
  }

  TEST_CASE("stream") {
    auto value = SequencedValue(std::string("hello world"), Sequence(1));
    REQUIRE(to_string(value) == "(hello world 1)");
    test_round_trip_shuttle(value);
  }
}
