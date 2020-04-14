#include <doctest/doctest.h>
#include "Beam/Queries/SequencedValue.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("SequencedValue") {
  TEST_CASE("default_constructor") {
    auto intValue = SequencedValue<int>();
    REQUIRE(intValue.GetSequence() == Sequence());
    REQUIRE(intValue.GetValue() == 0);
    auto stringValue = SequencedValue<std::string>();
    REQUIRE(stringValue.GetSequence() == Sequence());
    REQUIRE(stringValue.GetValue() == "");
  }

  TEST_CASE("value_and_sequence_constructor") {
    auto value = SequencedValue(std::string("hello world"), Sequence(1));
    REQUIRE(value.GetValue() == "hello world");
    REQUIRE(value.GetSequence() == Sequence(1));
  }

  TEST_CASE("dereference") {
    auto intValue = SequencedValue(123, Sequence(1));
    REQUIRE(*intValue == 123);
    auto stringValue = SequencedValue(std::string("hello world"), Sequence(1));
    REQUIRE(*stringValue == "hello world");
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

  TEST_CASE("make_sequenced_value") {
    auto value = SequencedValue(321, Sequence(1));
    REQUIRE(value.GetValue() == 321);
    REQUIRE(value.GetSequence() == Sequence(1));
  }
}
