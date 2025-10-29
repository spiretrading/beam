#include <stdexcept>
#include <string>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/Queues/MultiUpdateTablePublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("MultiUpdateTablePublisher") {
  TEST_CASE("push_single_update_and_snapshot") {
    auto publisher = MultiUpdateTablePublisher<int, std::string>();
    auto queue =
      std::make_shared<Queue<std::vector<KeyValuePair<int, std::string>>>>();
    auto snapshot = optional<std::unordered_map<int, std::string>>();
    publisher.monitor(queue, out(snapshot));
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->empty());
    publisher.push(1, std::string("one"));
    auto popped = queue->pop();
    REQUIRE(popped.size() == 1);
    REQUIRE(popped.front().m_key == 1);
    REQUIRE(popped.front().m_value == "one");
    auto current = publisher.get_snapshot();
    REQUIRE(current.has_value());
    REQUIRE(current->size() == 1);
    REQUIRE(current->at(1) == "one");
  }

  TEST_CASE("push_vector_updates_and_table_merge") {
    auto publisher = MultiUpdateTablePublisher<int, std::string>();
    auto queue =
      std::make_shared<Queue<std::vector<KeyValuePair<int, std::string>>>>();
    publisher.monitor(queue);
    auto updates = std::vector<KeyValuePair<int, std::string>>();
    updates.push_back(KeyValuePair(1, std::string("a")));
    updates.push_back(KeyValuePair(2, std::string("b")));
    publisher.push(updates);
    auto batch = queue->pop();
    auto map = std::unordered_map<int, std::string>();
    for(auto& p : batch) {
      map[p.m_key] = p.m_value;
    }
    REQUIRE(map.size() == 2);
    REQUIRE(map.at(1) == "a");
    REQUIRE(map.at(2) == "b");
    auto updates2 = std::vector<KeyValuePair<int, std::string>>();
    updates2.push_back(KeyValuePair(1, std::string("a2")));
    updates2.push_back(KeyValuePair(3, std::string("c")));
    publisher.push(updates2);
    auto batch2 = queue->pop();
    auto map2 = std::unordered_map<int, std::string>();
    for(auto& p : batch2) {
      map2[p.m_key] = p.m_value;
    }
    REQUIRE(map2.size() == 2);
    REQUIRE(map2.at(1) == "a2");
    REQUIRE(map2.at(3) == "c");
    auto snapshot = publisher.get_snapshot();
    REQUIRE(snapshot.has_value());
    REQUIRE(snapshot->size() == 3);
    REQUIRE(snapshot->at(1) == "a2");
    REQUIRE(snapshot->at(2) == "b");
    REQUIRE(snapshot->at(3) == "c");
  }

  TEST_CASE("monitor_with_no_snapshot_pushes_current_table") {
    auto publisher = MultiUpdateTablePublisher<int, std::string>();
    publisher.push(1, std::string("one"));
    publisher.push(2, std::string("two"));
    auto queue =
      std::make_shared<Queue<std::vector<KeyValuePair<int, std::string>>>>();
    publisher.monitor(queue);
    auto batch = queue->pop();
    auto map = std::unordered_map<int, std::string>();
    for(auto& p : batch) {
      map[p.m_key] = p.m_value;
    }
    REQUIRE(map.size() == 2);
    REQUIRE(map.at(1) == "one");
    REQUIRE(map.at(2) == "two");
  }

  TEST_CASE("push_empty_vector_is_noop") {
    auto publisher = MultiUpdateTablePublisher<int, std::string>();
    auto queue =
      std::make_shared<Queue<std::vector<KeyValuePair<int, std::string>>>>();
    publisher.monitor(queue);
    auto empty_updates = std::vector<KeyValuePair<int, std::string>>();
    publisher.push(empty_updates);
    auto opt = queue->try_pop();
    REQUIRE(!opt);
  }

  TEST_CASE("close_forwards_to_underlying_publisher") {
    auto publisher = MultiUpdateTablePublisher<int, std::string>();
    auto queue =
      std::make_shared<Queue<std::vector<KeyValuePair<int, std::string>>>>();
    publisher.monitor(queue);
    publisher.close(std::runtime_error("closed"));
    REQUIRE_THROWS_AS(queue->pop(), std::runtime_error);
  }

  TEST_CASE("with_noarg_synchronizes") {
    auto publisher = MultiUpdateTablePublisher<int, std::string>();
    auto flag = false;
    publisher.with([&] {
      flag = true;
    });
    REQUIRE(flag);
  }
}
