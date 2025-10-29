#include <doctest/doctest.h>
#include <boost/optional/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include "Beam/Serialization/ShuttleOptional.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

TEST_SUITE("ShuttleOptional") {
  TEST_CASE("none_int") {
    test_round_trip_shuttle(optional<int>());
  }

  TEST_CASE("some_int") {
    test_round_trip_shuttle(optional<int>(123));
  }

  TEST_CASE("none_string") {
    test_round_trip_shuttle(optional<std::string>());
  }

  TEST_CASE("some_string") {
    test_round_trip_shuttle(optional<std::string>("hello"));
  }

  TEST_CASE("bool_values") {
    test_round_trip_shuttle(optional<bool>(true));
    test_round_trip_shuttle(optional<bool>(false));
  }

  TEST_CASE("struct_value") {
    test_round_trip_shuttle(
      optional<StructWithFreeShuttle>(StructWithFreeShuttle('a', 2, 3.14)));
  }

  TEST_CASE("class_with_send_receive_value") {
    test_round_trip_shuttle(optional<ClassWithSendReceiveMethods>(
      ClassWithSendReceiveMethods('b', 10, 2.5)));
  }
}
