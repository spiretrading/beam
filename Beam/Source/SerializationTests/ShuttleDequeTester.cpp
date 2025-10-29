#include <deque>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleDeque.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleDeque") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::deque<int>());
  }

  TEST_CASE("int_sequence") {
    test_round_trip_shuttle(std::deque{5, 4, 3, 2, 1});
  }

  TEST_CASE("double_sequence") {
    test_round_trip_shuttle(std::deque{1.1, 2.2, 3.3, 4.4});
  }

  TEST_CASE("string_sequence") {
    test_round_trip_shuttle(std::deque<std::string>{"hello", ""});
  }

  TEST_CASE("single_element") {
    auto value = std::deque<int>();
    value.push_back(42);
    test_round_trip_shuttle(value);
  }

  TEST_CASE("struct_sequence") {
    auto value = std::deque<StructWithFreeShuttle>();
    value.push_back(StructWithFreeShuttle('x', 7, 1.23));
    test_round_trip_shuttle(value);
  }

  TEST_CASE("class_with_send_receive_sequence") {
    auto value = std::deque<ClassWithSendReceiveMethods>();
    value.push_back(ClassWithSendReceiveMethods('b', 10, 2.5));
    value.push_back(ClassWithSendReceiveMethods('c', 11, 3.5));
    test_round_trip_shuttle(value);
  }
}
