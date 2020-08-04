#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SequencedValuePublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;
using namespace Beam::Queries;

namespace {
  using TestQuery = BasicQuery<int>;
  using TestSequencedValuePublisher = SequencedValuePublisher<
    TestQuery, std::string>;

  struct PublisherEntry {
    std::shared_ptr<Queue<SequencedValue<std::string>>> m_queue;
    TestSequencedValuePublisher m_publisher;

    PublisherEntry(const TestQuery& query)
      : m_queue(std::make_shared<Queue<SequencedValue<std::string>>>()),
        m_publisher(query, Translate(query.GetFilter()), m_queue) {}
  };

  void ExpectValue(Queue<SequencedValue<std::string>>& queue,
      const SequencedValue<std::string>& expectedValue) {
    auto value = queue.Pop();
    REQUIRE(value == expectedValue);
  }

  auto MakePublisher(const TestQuery& query) {
    return std::make_unique<PublisherEntry>(query);
  }

  void InitializeSnapshot(PublisherEntry& publisher,
      const std::vector<SequencedValue<std::string>>& snapshot) {
    auto pushSnapshot = snapshot;
    publisher.m_publisher.BeginSnapshot();
    publisher.m_publisher.PushSnapshot(pushSnapshot.begin(),
      pushSnapshot.end());
    publisher.m_publisher.EndSnapshot(123);
    for(auto& value : snapshot) {
      ExpectValue(*publisher.m_queue, value);
    }
  }
}

TEST_SUITE("SequencedValuePublisher") {
  TEST_CASE("publish_with_total_range") {
    auto query = TestQuery();
    query.SetRange(Range::Total());
    auto publisher = MakePublisher(query);
    InitializeSnapshot(*publisher, {});
    auto helloValue = SequencedValue(std::string("hello"), Sequence(3));
    publisher->m_publisher.Push(helloValue);
    ExpectValue(*publisher->m_queue, helloValue);
    auto worldValue = SequencedValue(std::string("world"), Sequence(3));
    publisher->m_publisher.Push(worldValue);
    REQUIRE(!publisher->m_queue->TryPop());
    auto goodbyeValue = SequencedValue(std::string("goodbye"), Sequence(2));
    publisher->m_publisher.Push(goodbyeValue);
    REQUIRE(!publisher->m_queue->TryPop());
    auto skyValue = SequencedValue(std::string("goodbye"), Sequence(4));
    publisher->m_publisher.Push(skyValue);
    ExpectValue(*publisher->m_queue, skyValue);
  }

  TEST_CASE("snapshot_with_total_range") {
    auto query = TestQuery();
    query.SetRange(Range::Total());
    auto publisher = MakePublisher(query);
    auto snapshot = std::vector<SequencedValue<std::string>>();
    snapshot.push_back(SequencedValue("hello", Sequence(3)));
    InitializeSnapshot(*publisher, snapshot);
    auto worldValue = SequencedValue(std::string("world"), Sequence(3));
    publisher->m_publisher.Push(worldValue);
    REQUIRE(!publisher->m_queue->TryPop());
    auto goodbyeValue = SequencedValue(std::string("goodbye"), Sequence(2));
    publisher->m_publisher.Push(goodbyeValue);
    REQUIRE(!publisher->m_queue->TryPop());
    auto skyValue = SequencedValue(std::string("goodbye"), Sequence(4));
    publisher->m_publisher.Push(skyValue);
    ExpectValue(*publisher->m_queue, skyValue);
  }
}
