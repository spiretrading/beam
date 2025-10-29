#include <string>
#include <Aspen/Constant.hpp>
#include <Aspen/Queue.hpp>
#include <Aspen/Trigger.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/QueryReactor.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Aspen;
using namespace Beam;

TEST_SUITE("QueryReactor") {
  TEST_CASE("query_reactor_immediate_values") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Aspen::Trigger([&] {
      commits.push(true);
    });
    Aspen::Trigger::set_trigger(trigger);
    auto submission_count = 0;
    auto submission = [&] (const auto& query, auto queue) {
      ++submission_count;
      REQUIRE(query.get_index() == "test_index");
      queue->push(100);
      queue->push(200);
      queue->push(300);
      queue->close();
    };
    auto query = make_current_query(std::string("test_index"));
    auto reactor = query_reactor<int>(submission, query);
    auto sequence = 0;
    auto state = reactor.commit(sequence);
    ++sequence;
    auto received_values = std::vector<int>();
    while(true) {
      if(has_evaluation(state)) {
        received_values.push_back(reactor.eval());
      }
      if(is_complete(state)) {
        break;
      } else if(!has_continuation(state)) {
        commits.pop();
      }
      state = reactor.commit(sequence);
      ++sequence;
    }
    REQUIRE(submission_count == 1);
    REQUIRE(received_values == std::vector{100, 200, 300});
    Aspen::Trigger::set_trigger(nullptr);
  }

  TEST_CASE("query_reactor_empty_result") {
    auto commits = Beam::Queue<bool>();
    auto trigger = Aspen::Trigger([&] {
      commits.push(true);
    });
    Aspen::Trigger::set_trigger(trigger);
    auto submission = [&] (const auto& query, auto queue) {
      REQUIRE(query.get_index() == 42);
      queue->close();
    };
    auto query = make_current_query(42);
    auto reactor = query_reactor<std::string>(submission, query);
    auto sequence = 0;
    auto state = reactor.commit(sequence);
    ++sequence;
    while(true) {
      REQUIRE(!has_evaluation(state));
      if(is_complete(state)) {
        break;
      } else if(!has_continuation(state)) {
        commits.pop();
      }
      state = reactor.commit(sequence);
      ++sequence;
    }
    Aspen::Trigger::set_trigger(nullptr);
  }
}
