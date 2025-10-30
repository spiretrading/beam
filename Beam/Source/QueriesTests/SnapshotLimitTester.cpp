#include <doctest/doctest.h>
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("SnapshotLimit") {
  TEST_CASE("default_constructor") {
    auto limit = SnapshotLimit();
    REQUIRE(limit.get_size() == 0);
    REQUIRE(limit.get_type() == SnapshotLimit::Type::HEAD);
  }

  TEST_CASE("constructor") {
    auto head_limit = SnapshotLimit(SnapshotLimit::Type::HEAD, 123);
    REQUIRE(head_limit.get_type() == SnapshotLimit::Type::HEAD);
    REQUIRE(head_limit.get_size() == 123);
    auto negative_head_limit = SnapshotLimit(SnapshotLimit::Type::HEAD, -123);
    REQUIRE(negative_head_limit.get_type() == SnapshotLimit::Type::HEAD);
    REQUIRE(negative_head_limit.get_size() == 0);
    auto empty_head_limit = SnapshotLimit(SnapshotLimit::Type::HEAD, 0);
    REQUIRE(empty_head_limit.get_type() == SnapshotLimit::Type::HEAD);
    REQUIRE(empty_head_limit.get_size() == 0);
    auto tail_limit = SnapshotLimit(SnapshotLimit::Type::TAIL, 123);
    REQUIRE(tail_limit.get_type() == SnapshotLimit::Type::TAIL);
    REQUIRE(tail_limit.get_size() == 123);
    auto negative_tail_limit = SnapshotLimit(SnapshotLimit::Type::TAIL, -123);
    REQUIRE(negative_tail_limit.get_type() == SnapshotLimit::Type::HEAD);
    REQUIRE(negative_tail_limit.get_size() == 0);
    auto empty_tail_limit = SnapshotLimit(SnapshotLimit::Type::TAIL, 0);
    REQUIRE(empty_tail_limit.get_type() == SnapshotLimit::Type::HEAD);
    REQUIRE(empty_tail_limit.get_size() == 0);
  }

  TEST_CASE("none_snapshot_limit") {
    REQUIRE(SnapshotLimit::NONE.get_size() == 0);
  }

  TEST_CASE("unlimited_snapshot_limit") {
    REQUIRE(
      SnapshotLimit::UNLIMITED.get_size() == std::numeric_limits<int>::max());
  }

  TEST_CASE("equals_operator") {
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 123));
    REQUIRE(SnapshotLimit(
      SnapshotLimit::Type::HEAD, std::numeric_limits<int>::max()) ==
        SnapshotLimit(
          SnapshotLimit::Type::TAIL, std::numeric_limits<int>::max()));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 123));
    REQUIRE(SnapshotLimit(
      SnapshotLimit::Type::TAIL, std::numeric_limits<int>::max()) ==
        SnapshotLimit(
          SnapshotLimit::Type::HEAD, std::numeric_limits<int>::max()));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 1) ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 456)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 1) ==
      SnapshotLimit(SnapshotLimit::Type::HEAD, 1)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) ==
      SnapshotLimit(SnapshotLimit::Type::TAIL, 456)));
  }

  TEST_CASE("not_equals_operator") {
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 0) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 123)));
    REQUIRE(!(SnapshotLimit(
      SnapshotLimit::Type::HEAD, std::numeric_limits<int>::max()) !=
        SnapshotLimit(
          SnapshotLimit::Type::TAIL, std::numeric_limits<int>::max())));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 123)));
    REQUIRE(!(SnapshotLimit(
      SnapshotLimit::Type::TAIL, std::numeric_limits<int>::max()) !=
        SnapshotLimit(
          SnapshotLimit::Type::HEAD, std::numeric_limits<int>::max())));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 1) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 456));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 1) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 1));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 456));
  }

  TEST_CASE("stream") {
    auto head_limit = SnapshotLimit::from_head(123);
    REQUIRE(to_string(head_limit) == "(HEAD 123)");
    auto tail_limit = SnapshotLimit::from_tail(123);
    REQUIRE(to_string(tail_limit) == "(TAIL 123)");
    REQUIRE(to_string(SnapshotLimit::NONE) == "None");
    REQUIRE(to_string(SnapshotLimit::UNLIMITED) == "Unlimited");
    test_round_trip_shuttle(head_limit);
    test_round_trip_shuttle(tail_limit);
    test_round_trip_shuttle(SnapshotLimit::Type::HEAD);
    test_round_trip_shuttle(SnapshotLimit::Type::TAIL);
  }
}
