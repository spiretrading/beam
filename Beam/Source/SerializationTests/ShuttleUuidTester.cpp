#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleUuid.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost::uuids;

TEST_SUITE("ShuttleUuid") {
  TEST_CASE("nil_uuid") {
    test_round_trip_shuttle(uuid());
  }

  TEST_CASE("simple_uuid") {
    test_round_trip_shuttle(
      string_generator()("01234567-89ab-cdef-0123-456789abcdef"));
  }

  TEST_CASE("uppercase_uuid") {
    test_round_trip_shuttle(
      string_generator()("01234567-89AB-CDEF-0123-456789ABCDEF"));
  }

  TEST_CASE("another_uuid") {
    test_round_trip_shuttle(
      string_generator()("123e4567-e89b-12d3-a456-426655440000"));
  }
}
