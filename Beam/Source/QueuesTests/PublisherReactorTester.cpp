#include <vector>
#include <Aspen/CommitFlag.hpp>
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
    auto flag = CommitFlag();
    flag.set_trigger(&trigger);
    auto scope = CommitFlagScope(flag);
    auto publisher = std::make_shared<SequencePublisher<int>>();
    auto reactor = publisher_reactor(publisher);
    REQUIRE(reactor.commit(0) == State::NONE);
    flag.clear();
    publisher->close();
    commits.pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }
}
