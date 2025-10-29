#include <string>
#include <unordered_map>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleUnorderedMap.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleUnorderedMap") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::unordered_map<int, int>());
  }

  TEST_CASE("int_map") {
    test_round_trip_shuttle(
      std::unordered_map{std::pair(5, 4), std::pair(3, 2), std::pair(1, 0)});
  }

  TEST_CASE("string_key_map") {
    test_round_trip_shuttle(std::unordered_map{
      std::pair(std::string("hello"), 1), std::pair(std::string(""), 0)});
  }

  TEST_CASE("single_element") {
    test_round_trip_shuttle(
      std::unordered_map{std::pair(42, std::string("value"))});
  }

  TEST_CASE("struct_value_map") {
    test_round_trip_shuttle(std::unordered_map{
      std::pair(1, StructWithFreeShuttle('x', 7, 1.23)),
      std::pair(2, StructWithFreeShuttle('y', 8, 4.56))});
  }

  TEST_CASE("class_with_send_receive_map") {
    test_round_trip_shuttle(std::unordered_map{
      std::pair(1, ClassWithSendReceiveMethods('b', 10, 2.5)),
      std::pair(2, ClassWithSendReceiveMethods('c', 11, 3.5))});
  }
}
