#include <doctest/doctest.h>
#include "Beam/Queries/Sequencer.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace {
  struct Simple {
    ptime m_timestamp;
    int m_value;
  };
}

TEST_SUITE("Sequencer") {
  TEST_CASE("increment_next_sequence_same_partition") {
    auto timestamp = time_from_string("2020-01-01 01:00:00");
    auto initial = to_sequence(timestamp);
    auto sequencer = Sequencer(initial);
    auto sequence_a = sequencer.increment_next_sequence(
      time_from_string("2020-01-01 12:00:00"));
    REQUIRE(sequence_a.get_ordinal() == initial.get_ordinal());
    auto sequence_b = sequencer.increment_next_sequence(
      time_from_string("2020-01-01 13:00:00"));
    REQUIRE(sequence_b.get_ordinal() == initial.get_ordinal() + 1);
  }

  TEST_CASE("increment_next_sequence_new_partition") {
    auto initial_timestamp = time_from_string("2020-01-01 00:00:00");
    auto initial = to_sequence(initial_timestamp);
    auto sequencer = Sequencer(initial);
    auto new_timestamp = time_from_string("2020-01-02 00:00:00");
    auto sequence = sequencer.increment_next_sequence(new_timestamp);
    REQUIRE(sequence.get_ordinal() == to_sequence(new_timestamp).get_ordinal());
    auto next_sequence  = sequencer.increment_next_sequence(new_timestamp);
    REQUIRE(next_sequence.get_ordinal() ==
      to_sequence(new_timestamp).get_ordinal() + 1);
  }

  TEST_CASE("make_sequenced_value_encodes_timestamp_and_preserves_index") {
    auto timestamp = time_from_string("2021-05-03 08:00:00");
    auto initial = to_sequence(timestamp);
    auto sequencer = Sequencer(initial);
    auto value = Simple(timestamp, 99);
    auto sequenced_value = sequencer.make_sequenced_value(value, 7);
    REQUIRE(
      sequenced_value.get_sequence().get_ordinal() == initial.get_ordinal());
    REQUIRE(sequenced_value.get_value().get_index() == 7);
    REQUIRE(sequenced_value.get_value().get_value().m_value == 99);
  }

  TEST_CASE("to_sequenced_value_removes_index") {
    auto timestamp = time_from_string("2021-05-03 08:00:00");
    auto initial = to_sequence(timestamp);
    auto sequencer = Sequencer(initial);
    auto value = Simple(timestamp, 42);
    auto indexed = sequencer.make_sequenced_value(value, 13);
    auto plain = to_sequenced_value(indexed);
    REQUIRE(plain.get_sequence() == indexed.get_sequence());
    REQUIRE(plain.get_value().m_value == value.m_value);
  }
}
