#include <doctest/doctest.h>
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("InterruptionPolicy") {
  TEST_CASE("stream") {
    REQUIRE(to_string(InterruptionPolicy::BREAK_QUERY) == "BREAK_QUERY");
    REQUIRE(to_string(InterruptionPolicy::RECOVER_DATA) == "RECOVER_DATA");
    REQUIRE(to_string(InterruptionPolicy::IGNORE_CONTINUE) == "IGNORE_CONTINUE");
    REQUIRE(to_string(static_cast<InterruptionPolicy>(-1)) == "NONE");
    test_round_trip_shuttle(InterruptionPolicy::BREAK_QUERY);
  }

  TEST_CASE("receiving_invalid_value_throws_and_sets_break_query") {
    auto sender = JsonSender<SharedBuffer>();
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.send(99);
    auto receiver = JsonReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto policy = InterruptionPolicy(InterruptionPolicy::RECOVER_DATA);
    REQUIRE_THROWS_AS(receiver.receive(policy), SerializationException);
    REQUIRE(policy == InterruptionPolicy::BREAK_QUERY);
  }
}
