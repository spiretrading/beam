#include <doctest/doctest.h>
#include "Beam/Queues/AggregateQueueReader.hpp"

using namespace Beam;

namespace {
  bool IsPermutation(const std::vector<int>& a, const std::vector<int>& b) {
    return std::is_permutation(a.begin(), a.end(), b.begin(), b.end());
  }
}

TEST_SUITE("AggregateQueueReader") {
  TEST_CASE("empty") {
    auto queue = AggregateQueueReader<int>({});
    REQUIRE_THROWS_AS(queue.Pop(), PipeBrokenException);
  }

  TEST_CASE("single") {
    auto source = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source);
    auto queue = AggregateQueueReader(std::move(queues));
    source->Push(123);
    REQUIRE(queue.Pop() == 123);
    source->Push(321);
    source->Break();
    REQUIRE(queue.Pop() == 321);
    REQUIRE_THROWS_AS(queue.Pop(), PipeBrokenException);
  }

  TEST_CASE("double") {
    auto source1 = std::make_shared<Queue<int>>();
    auto source2 = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source1);
    queues.push_back(source2);
    auto queue = AggregateQueueReader(std::move(queues));
    source2->Push(3);
    source1->Push(1);
    source2->Push(4);
    source1->Push(5);
    source1->Push(8);
    source1->Push(1);
    source1->Break();
    auto pops = std::vector<int>();
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    source2->Push(2);
    source2->Push(6);
    source2->Push(1);
    source2->Push(3);
    source2->Break();
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    pops.push_back(queue.Pop());
    REQUIRE(IsPermutation(pops, {3, 1, 4, 5, 8, 1, 2, 6, 1, 3}));
    REQUIRE_THROWS_AS(queue.Pop(), PipeBrokenException);
  }

  TEST_CASE("break") {
    auto source1 = std::make_shared<Queue<int>>();
    auto source2 = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source1);
    queues.push_back(source2);
    auto queue = AggregateQueueReader(std::move(queues));
    source2->Push(3);
    source1->Push(1);
    queue.Pop();
    queue.Pop();
    queue.Break();
    REQUIRE(source1->IsBroken());
    REQUIRE(source2->IsBroken());
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
    REQUIRE(source1->IsBroken());
    REQUIRE(source2->IsBroken());
  }
}
