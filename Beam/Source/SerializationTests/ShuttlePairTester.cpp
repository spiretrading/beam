#include <utility>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttlePair.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttlePair") {
  TEST_CASE("int_int") {
    test_round_trip_shuttle(std::pair(1, 2));
  }

  TEST_CASE("string_int") {
    test_round_trip_shuttle(std::pair(std::string("hello"), 42));
  }

  TEST_CASE("empty_string_and_zero") {
    test_round_trip_shuttle(std::pair(std::string(""), 0));
  }

  TEST_CASE("struct_and_int") {
    test_round_trip_shuttle(std::pair(StructWithFreeShuttle('x', 7, 1.23), 9));
  }

  TEST_CASE("class_and_class") {
    test_round_trip_shuttle(std::pair(ClassWithSendReceiveMethods('b', 10, 2.5),
      ClassWithSendReceiveMethods('c', 11, 3.5)));
  }

  TEST_CASE("nested_pairs") {
    test_round_trip_shuttle(std::pair(1, std::pair(2, 3)));
  }
}
