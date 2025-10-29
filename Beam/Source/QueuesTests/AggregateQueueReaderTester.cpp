#include <doctest/doctest.h>
#include "Beam/Queues/AggregateQueueReader.hpp"

using namespace Beam;

namespace {
  bool is_permutation(
      const std::vector<int>& a, const std::vector<int>& b) {
    return std::is_permutation(a.begin(), a.end(), b.begin(), b.end());
  }
}

TEST_SUITE("AggregateQueueReader") {
  TEST_CASE("empty") {
    auto queue = AggregateQueueReader<int>({});
    REQUIRE_THROWS_AS(queue.pop(), PipeBrokenException);
  }

  TEST_CASE("single") {
    auto source = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source);
    auto queue = AggregateQueueReader(std::move(queues));
    source->push(123);
    REQUIRE(queue.pop() == 123);
    source->push(321);
    source->close();
    REQUIRE(queue.pop() == 321);
    REQUIRE_THROWS_AS(queue.pop(), PipeBrokenException);
  }

  TEST_CASE("double") {
    auto source1 = std::make_shared<Queue<int>>();
    auto source2 = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source1);
    queues.push_back(source2);
    auto queue = AggregateQueueReader(std::move(queues));
    source2->push(3);
    source1->push(1);
    source2->push(4);
    source1->push(5);
    source1->push(8);
    source1->push(1);
    source1->close();
    auto pops = std::vector<int>();
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    source2->push(2);
    source2->push(6);
    source2->push(1);
    source2->push(3);
    source2->close();
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    pops.push_back(queue.pop());
    REQUIRE(is_permutation(pops, {3, 1, 4, 5, 8, 1, 2, 6, 1, 3}));
    REQUIRE_THROWS_AS(queue.pop(), PipeBrokenException);
  }

  TEST_CASE("close") {
    auto source1 = std::make_shared<Queue<int>>();
    auto source2 = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source1);
    queues.push_back(source2);
    auto queue = AggregateQueueReader(std::move(queues));
    source2->push(3);
    source1->push(1);
    queue.pop();
    queue.pop();
    queue.close();
    REQUIRE(source1->is_broken());
    REQUIRE(source2->is_broken());
  }

  TEST_CASE("break_on_destruction") {
    auto source1 = std::make_shared<Queue<int>>();
    auto source2 = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source1);
    queues.push_back(source2);
    {
      auto queue = AggregateQueueReader(std::move(queues));
    }
    REQUIRE(source1->is_broken());
    REQUIRE(source2->is_broken());
  }
}
