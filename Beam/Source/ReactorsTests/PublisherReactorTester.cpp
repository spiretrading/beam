#include <vector>
#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/SequencePublisher.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Reactors;

TEST_SUITE("PublisherReactorTester") {
  TEST_CASE("empty") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Trigger(
      [&] {
        commits.Push(true);
      });
    Trigger::set_trigger(trigger);
    auto publisher = std::make_shared<SequencePublisher<int>>();
    auto reactor = PublisherReactor(publisher);
    REQUIRE(reactor.commit(0) == State::NONE);
    publisher->Break();
    commits.Pop();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    Trigger::set_trigger(nullptr);
  }
}
