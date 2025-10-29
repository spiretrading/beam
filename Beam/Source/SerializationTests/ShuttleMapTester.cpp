#include <map>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleMap.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleMap") {
  TEST_CASE("zero_length") {
    test_round_trip_shuttle(std::map<int, int>());
  }

  TEST_CASE("int_map") {
    test_round_trip_shuttle(std::map<int, int>{{5, 4}, {3, 2}, {1, 0}});
  }

  TEST_CASE("string_key_map") {
    test_round_trip_shuttle(std::map<std::string, int>{{"hello", 1}, {"", 0}});
  }

  TEST_CASE("single_element") {
    test_round_trip_shuttle(std::map<int, std::string>{{42, "value"}});
  }

  TEST_CASE("struct_value_map") {
    auto map = std::map<int, StructWithFreeShuttle>();
    map.emplace(1, StructWithFreeShuttle('x', 7, 1.23));
    map.emplace(2, StructWithFreeShuttle('y', 8, 4.56));
    test_round_trip_shuttle(map);
  }

  TEST_CASE("class_with_send_receive_map") {
    auto map = std::map<int, ClassWithSendReceiveMethods>();
    map.emplace(1, ClassWithSendReceiveMethods('b', 10, 2.5));
    map.emplace(2, ClassWithSendReceiveMethods('c', 11, 3.5));
    test_round_trip_shuttle(map);
  }
}
