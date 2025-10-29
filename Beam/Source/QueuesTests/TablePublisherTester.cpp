#include <doctest/doctest.h>
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/TablePublisher.hpp"

using namespace boost;
using namespace Beam;

TEST_SUITE("TablePublisher") {
  TEST_CASE("push_and_snapshot_and_queue_receive") {
    auto publisher = TablePublisher<int, std::string>();
    auto queue = std::make_shared<Queue<KeyValuePair<int, std::string>>>();
    auto snapshot = optional<std::unordered_map<int, std::string>>();
    publisher.monitor(queue, out(snapshot));
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->empty());
    publisher.push(1, "one");
    auto item = queue->pop();
    REQUIRE(item.m_key == 1);
    REQUIRE(item.m_value == "one");
    auto current = publisher.get_snapshot();
    REQUIRE(current.has_value());
    REQUIRE(current->size() == 1);
    REQUIRE(current->at(1) == "one");
  }

  TEST_CASE("monitor_pushes_existing_table_entries") {
    auto publisher = TablePublisher<int, std::string>();
    publisher.push(1, "one");
    publisher.push(2, "two");
    auto queue = std::make_shared<Queue<KeyValuePair<int, std::string>>>();
    publisher.monitor(queue);
    auto entries = std::vector<KeyValuePair<int, std::string>>();
    entries.push_back(queue->pop());
    entries.push_back(queue->pop());
    REQUIRE(
      std::ranges::contains(entries, KeyValuePair(1, std::string("one"))));
    REQUIRE(
      std::ranges::contains(entries, KeyValuePair(2, std::string("two"))));
  }

  TEST_CASE("erase_removes_key_and_publishes_erased_value") {
    auto publisher = TablePublisher<int, std::string>();
    auto queue = std::make_shared<Queue<KeyValuePair<int, std::string>>>();
    publisher.monitor(queue);
    publisher.push(1, "one");
    publisher.push(2, "two");
    queue->pop();
    queue->pop();
    publisher.erase(1);
    auto erased = queue->pop();
    REQUIRE(erased.m_key == 1);
    REQUIRE(erased.m_value == "one");
    auto snapshot = publisher.get_snapshot();
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->count(1) == 0);
    REQUIRE(snapshot->count(2) == 1);
  }

  TEST_CASE("push_rvalue_overload") {
    auto publisher = TablePublisher<int, std::string>();
    auto queue = std::make_shared<Queue<KeyValuePair<int, std::string>>>();
    publisher.monitor(queue);
    auto pair = KeyValuePair<int, std::string>();
    pair.m_key = 3;
    pair.m_value = "three";
    publisher.push(std::move(pair));
    auto popped = queue->pop();
    REQUIRE(popped.m_key == 3);
    REQUIRE(popped.m_value == "three");
    auto snapshot = publisher.get_snapshot();
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->at(3) == "three");
  }

  TEST_CASE("close_closes_underlying_queues") {
    auto publisher = TablePublisher<int, std::string>();
    auto queue = std::make_shared<Queue<KeyValuePair<int, std::string>>>();
    publisher.monitor(queue);
    publisher.close(std::runtime_error("closed"));
    REQUIRE_THROWS_AS(queue->pop(), std::runtime_error);
    auto queue2 = std::make_shared<Queue<KeyValuePair<int, std::string>>>();
    publisher.monitor(queue2);
    REQUIRE_THROWS_AS(queue2->pop(), std::runtime_error);
  }

  TEST_CASE("with_noarg_synchronizes") {
    auto publisher = TablePublisher<int, std::string>();
    auto flag = false;
    publisher.with([&] {
      flag = true;
    });
    REQUIRE(flag);
  }
}
