#include <vector>
#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Queues/PublisherReactor.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/SequencePublisher.hpp"

using namespace Aspen;
using namespace Beam;

TEST_SUITE("PublisherReactorTester") {
  TEST_CASE("empty") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger([&] {
      commits.push(true);
    });
    Trigger::set_trigger(trigger);
    auto publisher = std::make_shared<SequencePublisher<int>>();
    auto reactor = publisher_reactor(publisher);
    REQUIRE(reactor.commit(0) == State::NONE);
    publisher->close();
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }
}
