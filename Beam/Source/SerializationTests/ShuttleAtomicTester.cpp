#include <atomic>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleAtomic.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

TEST_SUITE("ShuttleAtomic") {
  TEST_CASE("none_int") {
    test_round_trip_shuttle(std::atomic<int>());
  }

  TEST_CASE("some_int") {
    test_round_trip_shuttle(std::atomic<int>(123));
  }

  TEST_CASE("bool_values") {
    test_round_trip_shuttle(std::atomic<bool>(true));
    test_round_trip_shuttle(std::atomic<bool>(false));
  }

  TEST_CASE("struct_value") {
    test_round_trip_shuttle(
      std::atomic<StructWithFreeShuttle>(StructWithFreeShuttle('a', 2, 3.14)),
      [] (auto&& received) {
        REQUIRE(received.load() == StructWithFreeShuttle('a', 2, 3.14));
      });
  }

  TEST_CASE("class_with_send_receive_value") {
    test_round_trip_shuttle(std::atomic<ClassWithSendReceiveMethods>(
      ClassWithSendReceiveMethods('b', 10, 2.5)), [] (auto&& received) {
        REQUIRE(received.load() ==
          ClassWithSendReceiveMethods('b', 10, 2.5));
      });
  }
}
