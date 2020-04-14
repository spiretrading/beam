#include <doctest/doctest.h>
#include "Beam/Queries/Sequence.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("Sequence") {
  TEST_CASE("default_constructor") {
    auto sequence = Sequence();
    REQUIRE(sequence.GetOrdinal() == 0);
  }

  TEST_CASE("ordinal_constructor") {
    auto sequenceA = Sequence(1);
    REQUIRE(sequenceA.GetOrdinal() == 1);
    auto sequenceB = Sequence(5);
    REQUIRE(sequenceB.GetOrdinal() == 5);
    auto sequenceC = Sequence(std::numeric_limits<std::uint64_t>::max());
    REQUIRE(sequenceC.GetOrdinal() ==
      std::numeric_limits<std::uint64_t>::max());
  }

  TEST_CASE("first_sequence") {
    auto first = Sequence::First();
    REQUIRE(first.GetOrdinal() == 0);
  }

  TEST_CASE("last_sequence") {
    auto last = Sequence::Last();
    REQUIRE(last.GetOrdinal() == std::numeric_limits<std::uint64_t>::max());
  }

  TEST_CASE("less_than_operator") {
    REQUIRE(!(Sequence(0) < Sequence(0)));
    REQUIRE(Sequence(0) < Sequence(1));
    REQUIRE(!(Sequence(1) < Sequence(0)));
    REQUIRE(!(Sequence(1) < Sequence(1)));
  }

  TEST_CASE("less_than_or_equal_operator") {
    REQUIRE(Sequence(0) <= Sequence(0));
    REQUIRE(Sequence(0) <= Sequence(1));
    REQUIRE(!(Sequence(1) <= Sequence(0)));
    REQUIRE(Sequence(1) <= Sequence(1));
  }

  TEST_CASE("equals_operator") {
    REQUIRE(Sequence(0) == Sequence(0));
    REQUIRE(!(Sequence(0) == Sequence(1)));
    REQUIRE(!(Sequence(1) == Sequence(0)));
    REQUIRE(Sequence(1) == Sequence(1));
  }

  TEST_CASE("not_equals_operator") {
    REQUIRE(!(Sequence(0) != Sequence(0)));
    REQUIRE(Sequence(0) != Sequence(1));
    REQUIRE(Sequence(1) != Sequence(0));
    REQUIRE(!(Sequence(1) != Sequence(1)));
  }

  TEST_CASE("greater_than_or_equal_operator") {
    REQUIRE(Sequence(0) >= Sequence(0));
    REQUIRE(!(Sequence(0) >= Sequence(1)));
    REQUIRE(Sequence(1) >= Sequence(0));
    REQUIRE(Sequence(1) >= Sequence(1));
  }

  TEST_CASE("greater_than_operator") {
    REQUIRE(!(Sequence(0) > Sequence(0)));
    REQUIRE(!(Sequence(0) > Sequence(1)));
    REQUIRE(Sequence(1) > Sequence(0));
    REQUIRE(!(Sequence(1) > Sequence(1)));
  }

  TEST_CASE("increment") {
    REQUIRE(Increment(Sequence(0)) == Sequence(1));
    REQUIRE(Increment(Sequence(1)) == Sequence(2));
    REQUIRE(Increment(Sequence(2)) == Sequence(3));
    REQUIRE(Increment(Sequence::Last()) == Sequence::Last());
  }

  TEST_CASE("decrement") {
    REQUIRE(Decrement(Sequence(1)) == Sequence(0));
    REQUIRE(Decrement(Sequence(2)) == Sequence(1));
    REQUIRE(Decrement(Sequence(3)) == Sequence(2));
    REQUIRE(Decrement(Sequence::First()) == Sequence::First());
  }

  TEST_CASE("encoding_timestamp") {
    auto timestamp = ptime(date(1984, May, 6),
      hours(12) + minutes(44) + seconds(53));
    auto sequence = EncodeTimestamp(timestamp);
    auto encoding = static_cast<Sequence::Ordinal>(0b00111110000000101000110) <<
      (CHAR_BIT * sizeof(Sequence::Ordinal) - 23);
    REQUIRE(encoding == sequence.GetOrdinal());
    REQUIRE(DecodeTimestamp(Sequence{encoding}).date() == timestamp.date());
  }
}
