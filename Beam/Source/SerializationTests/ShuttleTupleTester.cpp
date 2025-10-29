#include <string>
#include <tuple>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleTuple.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleTuple") {
  TEST_CASE("empty_tuple") {
    test_round_trip_shuttle(std::tuple<>());
  }

  TEST_CASE("single_element") {
    test_round_trip_shuttle(std::tuple(123));
  }

  TEST_CASE("mixed_types") {
    test_round_trip_shuttle(std::tuple(123, 3.14, std::string("hello")));
  }

  TEST_CASE("nested_tuple") {
    test_round_trip_shuttle(std::tuple(1, std::tuple(2, std::string("inner"))));
  }

  TEST_CASE("struct_and_class") {
    test_round_trip_shuttle(std::tuple(StructWithFreeShuttle('x', 7, 1.23),
      ClassWithSendReceiveMethods('b', 10, 2.5)));
  }

  TEST_CASE("tuple_of_tuples") {
    test_round_trip_shuttle(std::tuple(std::tuple(1, 2),
      std::tuple(std::string("a"), std::string("b")), std::tuple(3.14)));
  }
}
