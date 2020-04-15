#include <doctest/doctest.h>
#include "Beam/Queries/SnapshotLimit.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("SnapshotLimit") {
  TEST_CASE("default_constructor") {
    auto limit = SnapshotLimit();
    REQUIRE(limit.GetSize() == 0);
    REQUIRE(limit.GetType() == SnapshotLimit::Type::HEAD);
  }

  TEST_CASE("constructor") {
    auto headLimit = SnapshotLimit(SnapshotLimit::Type::HEAD, 123);
    REQUIRE(headLimit.GetType() == SnapshotLimit::Type::HEAD);
    REQUIRE(headLimit.GetSize() == 123);
    auto negativeHeadLimit = SnapshotLimit(SnapshotLimit::Type::HEAD, -123);
    REQUIRE(negativeHeadLimit.GetType() == SnapshotLimit::Type::HEAD);
    REQUIRE(negativeHeadLimit.GetSize() == 0);
    auto emptyHeadLimit = SnapshotLimit(SnapshotLimit::Type::HEAD, 0);
    REQUIRE(emptyHeadLimit.GetType() == SnapshotLimit::Type::HEAD);
    REQUIRE(emptyHeadLimit.GetSize() == 0);
    auto tailLimit = SnapshotLimit(SnapshotLimit::Type::TAIL, 123);
    REQUIRE(tailLimit.GetType() == SnapshotLimit::Type::TAIL);
    REQUIRE(tailLimit.GetSize() == 123);
    auto negativeTailLimit = SnapshotLimit(SnapshotLimit::Type::TAIL, -123);
    REQUIRE(negativeTailLimit.GetType() == SnapshotLimit::Type::HEAD);
    REQUIRE(negativeTailLimit.GetSize() == 0);
    auto emptyTailLimit = SnapshotLimit(SnapshotLimit::Type::TAIL, 0);
    REQUIRE(emptyTailLimit.GetType() == SnapshotLimit::Type::HEAD);
    REQUIRE(emptyTailLimit.GetSize() == 0);
  }

  TEST_CASE("none_snapshot_limit") {
    REQUIRE(SnapshotLimit::None().GetSize() == 0);
  }

  TEST_CASE("unlimited_snapshot_limit") {
    REQUIRE(SnapshotLimit::Unlimited().GetSize() ==
      std::numeric_limits<int>::max());
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
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::HEAD,
      std::numeric_limits<int>::max()) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL,
      std::numeric_limits<int>::max())));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 0)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 0) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 0)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 123)));
    REQUIRE(!(SnapshotLimit(SnapshotLimit::Type::TAIL,
      std::numeric_limits<int>::max()) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD,
      std::numeric_limits<int>::max())));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 1) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 1));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::HEAD, 123) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 456));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 1) !=
      SnapshotLimit(SnapshotLimit::Type::HEAD, 1));
    REQUIRE(SnapshotLimit(SnapshotLimit::Type::TAIL, 123) !=
      SnapshotLimit(SnapshotLimit::Type::TAIL, 456));
  }
}
