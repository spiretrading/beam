#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("InterruptionPolicy") {
  TEST_CASE("stream") {
    auto ss = std::stringstream();
    ss << InterruptionPolicy::BREAK_QUERY;
    REQUIRE(ss.str() == "BREAK_QUERY");
    ss.str("");
    ss << InterruptionPolicy::RECOVER_DATA;
    REQUIRE(ss.str() == "RECOVER_DATA");
    ss.str("");
    ss << InterruptionPolicy::IGNORE_CONTINUE;
    REQUIRE(ss.str() == "IGNORE_CONTINUE");
    ss.str("");
    ss << static_cast<InterruptionPolicy>(-1);
    REQUIRE(ss.str() == "NONE");
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
