#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Queries/Sequence.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("Sequence") {
  TEST_CASE("default_constructor") {
    auto sequence = Sequence();
    REQUIRE(sequence.get_ordinal () == 0);
  }

  TEST_CASE("ordinal_constructor") {
    auto sequence_a = Sequence(1);
    REQUIRE(sequence_a.get_ordinal () == 1);
    auto sequence_b = Sequence(5);
    REQUIRE(sequence_b.get_ordinal () == 5);
    auto sequence_c = Sequence(std::numeric_limits<std::uint64_t>::max());
    REQUIRE(
      sequence_c.get_ordinal () == std::numeric_limits<std::uint64_t>::max());
  }

  TEST_CASE("first_sequence") {
    auto first = Sequence::FIRST;
    REQUIRE(first.get_ordinal () == 0);
  }

  TEST_CASE("last_sequence") {
    auto last = Sequence::LAST;
    REQUIRE(last.get_ordinal () == std::numeric_limits<std::uint64_t>::max());
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
    REQUIRE(increment(Sequence(0)) == Sequence(1));
    REQUIRE(increment(Sequence(1)) == Sequence(2));
    REQUIRE(increment(Sequence(2)) == Sequence(3));
    REQUIRE(increment(Sequence::LAST) == Sequence::LAST);
  }

  TEST_CASE("decrement") {
    REQUIRE(decrement(Sequence(1)) == Sequence(0));
    REQUIRE(decrement(Sequence(2)) == Sequence(1));
    REQUIRE(decrement(Sequence(3)) == Sequence(2));
    REQUIRE(decrement(Sequence::FIRST) == Sequence::FIRST);
  }

  TEST_CASE("encoding_timestamp") {
    auto timestamp =
      ptime(date(1984, May, 6), hours(12) + minutes(44) + seconds(53));
    auto sequence = to_sequence(timestamp);
    auto encoding = static_cast<Sequence::Ordinal>(0b00111110000000101000110) <<
      (CHAR_BIT * sizeof(Sequence::Ordinal) - 23);
    REQUIRE(encoding == sequence.get_ordinal ());
    REQUIRE(decode_timestamp(Sequence(encoding)).date() == timestamp.date());
  }

  TEST_CASE("stream") {
    auto ss = std::stringstream();
    ss << Sequence(123);
    REQUIRE(ss.str() == "123");
    test_round_trip_shuttle(Sequence(543));
  }
}
