#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SequencedValuePublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

namespace {
  using TestQuery = BasicQuery<int>;
  using TestSequencedValuePublisher =
    SequencedValuePublisher<TestQuery, std::string>;

  struct PublisherEntry {
    std::shared_ptr<Queue<SequencedValue<std::string>>> m_queue;
    TestSequencedValuePublisher m_publisher;

    PublisherEntry(const TestQuery& query)
      : m_queue(std::make_shared<Queue<SequencedValue<std::string>>>()),
        m_publisher(query, translate(query.get_filter()), m_queue) {}
  };

  void expect(Queue<SequencedValue<std::string>>& queue,
      const SequencedValue<std::string>& expected) {
    auto value = queue.pop();
    REQUIRE(value == expected);
  }

  auto make_publisher(const TestQuery& query) {
    return std::make_unique<PublisherEntry>(query);
  }

  void initialize(PublisherEntry& publisher,
      const std::vector<SequencedValue<std::string>>& snapshot) {
    auto push_snapshot = snapshot;
    publisher.m_publisher.begin_snapshot();
    publisher.m_publisher.push_snapshot(
      push_snapshot.begin(), push_snapshot.end());
    publisher.m_publisher.end_snapshot(123);
    for(auto& value : snapshot) {
      expect(*publisher.m_queue, value);
    }
  }
}

TEST_SUITE("SequencedValuePublisher") {
  TEST_CASE("publish_with_total_range") {
    auto query = TestQuery();
    query.set_range(Range::TOTAL);
    auto publisher = make_publisher(query);
    initialize(*publisher, {});
    auto hello_value = SequencedValue(std::string("hello"), Sequence(3));
    publisher->m_publisher.push(hello_value);
    expect(*publisher->m_queue, hello_value);
    auto world_value = SequencedValue(std::string("world"), Sequence(3));
    publisher->m_publisher.push(world_value);
    REQUIRE(!publisher->m_queue->try_pop());
    auto goodbye_value = SequencedValue(std::string("goodbye"), Sequence(2));
    publisher->m_publisher.push(goodbye_value);
    REQUIRE(!publisher->m_queue->try_pop());
    auto sky_value = SequencedValue(std::string("goodbye"), Sequence(4));
    publisher->m_publisher.push(sky_value);
    expect(*publisher->m_queue, sky_value);
  }

  TEST_CASE("snapshot_with_total_range") {
    auto query = TestQuery();
    query.set_range(Range::TOTAL);
    auto publisher = make_publisher(query);
    auto snapshot = std::vector<SequencedValue<std::string>>();
    snapshot.push_back(SequencedValue(std::string("hello"), Sequence(3)));
    initialize(*publisher, snapshot);
    auto world_value = SequencedValue(std::string("world"), Sequence(3));
    publisher->m_publisher.push(world_value);
    REQUIRE(!publisher->m_queue->try_pop());
    auto goodbye_value = SequencedValue(std::string("goodbye"), Sequence(2));
    publisher->m_publisher.push(goodbye_value);
    REQUIRE(!publisher->m_queue->try_pop());
    auto sky_value = SequencedValue(std::string("goodbye"), Sequence(4));
    publisher->m_publisher.push(sky_value);
    expect(*publisher->m_queue, sky_value);
  }

  TEST_CASE("get_id_updated_on_end_snapshot") {
    auto query = TestQuery();
    query.set_range(Range::TOTAL);
    auto publisher = make_publisher(query);
    publisher->m_publisher.begin_snapshot();
    auto value = SequencedValue(std::string("a"), Sequence(1));
    publisher->m_publisher.push(value);
    REQUIRE(publisher->m_publisher.get_id() == -1);
    publisher->m_publisher.end_snapshot(77);
    REQUIRE(publisher->m_publisher.get_id() == 77);
  }

  TEST_CASE("filter_blocks_publication") {
    auto query = TestQuery();
    query.set_filter(ConstantExpression(false));
    query.set_range(Range::TOTAL);
    auto publisher = make_publisher(query);
    publisher->m_publisher.begin_snapshot();
    auto snapshot = std::vector<SequencedValue<std::string>>();
    snapshot.push_back(SequencedValue(std::string("hidden"), Sequence(5)));
    publisher->m_publisher.push_snapshot(snapshot.begin(), snapshot.end());
    publisher->m_publisher.end_snapshot(1);
    REQUIRE(!publisher->m_queue->try_pop());
  }

  TEST_CASE("begin_recovery_ignore_continue") {
    auto query = TestQuery();
    query.set_range(Range::TOTAL);
    query.set_interruption_policy(InterruptionPolicy::IGNORE_CONTINUE);
    auto publisher = make_publisher(query);
    auto recovery = publisher->m_publisher.begin_recovery();
    REQUIRE(recovery.get_range().get_start() == Sequence::PRESENT);
  }

  TEST_CASE("begin_recovery_recover_data") {
    auto query = TestQuery();
    query.set_range(Range(Sequence(2), Sequence(10)));
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    auto publisher = make_publisher(query);
    auto recovery = publisher->m_publisher.begin_recovery();
    REQUIRE(recovery.get_range().get_start() == query.get_range().get_start());
  }

  TEST_CASE("begin_recovery_recover_data_when_non_first") {
    auto query = TestQuery();
    query.set_range(Range::TOTAL);
    query.set_interruption_policy(InterruptionPolicy::RECOVER_DATA);
    auto publisher = make_publisher(query);
    publisher->m_publisher.begin_snapshot();
    auto value = SequencedValue(std::string("one"), Sequence(10));
    publisher->m_publisher.push(value);
    publisher->m_publisher.end_snapshot(2);
    auto recovery = publisher->m_publisher.begin_recovery();
    REQUIRE(recovery.get_range().get_start() == Sequence(11));
  }

  TEST_CASE("begin_recovery_break_policy") {
    auto query = TestQuery();
    query.set_range(Range::TOTAL);
    query.set_interruption_policy(InterruptionPolicy::BREAK_QUERY);
    auto publisher = make_publisher(query);
    REQUIRE_THROWS_AS(
      publisher->m_publisher.begin_recovery(), QueryInterruptedException);
  }
}
