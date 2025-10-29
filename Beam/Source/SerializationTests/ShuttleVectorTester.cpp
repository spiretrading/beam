#include <string>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleVector.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleVector") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::vector<int>());
  }

  TEST_CASE("int_sequence") {
    test_round_trip_shuttle(std::vector{5, 4, 3, 2, 1});
  }

  TEST_CASE("double_sequence") {
    test_round_trip_shuttle(std::vector{1.1, 2.2, 3.3, 4.4});
  }

  TEST_CASE("string_sequence") {
    test_round_trip_shuttle(std::vector<std::string>{"hello", ""});
  }

  TEST_CASE("single_element") {
    test_round_trip_shuttle(std::vector{42});
  }

  TEST_CASE("struct_sequence") {
    test_round_trip_shuttle(std::vector{
      StructWithFreeShuttle('x', 7, 1.23),
      StructWithFreeShuttle('y', 8, 4.56)});
  }

  TEST_CASE("class_with_send_receive_sequence") {
    test_round_trip_shuttle(std::vector{
      ClassWithSendReceiveMethods('b', 10, 2.5),
      ClassWithSendReceiveMethods('c', 11, 3.5)});
  }
}
