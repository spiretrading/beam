#include <doctest/doctest.h>
#include <boost/optional/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Queues/ScopedQueueWriter.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;
using namespace boost;

namespace {
  struct TestSnapshotPublisher : SnapshotPublisher<int, std::string> {
    optional<std::string> m_snapshot;
    mutable std::vector<ScopedQueueWriter<int>> m_queues;

    TestSnapshotPublisher() = default;

    void publish(int value) {
      std::erase_if(m_queues, [&] (auto& q) {
        try {
          q.push(value);
          return false;
        } catch(const std::exception&) {
          return true;
        }
      });
    }

    void with(const std::function<void (optional<const Snapshot&>)>& f)
        const override {
      if(m_snapshot) {
        f(*m_snapshot);
      } else {
        f(none);
      }
    }

    void monitor(ScopedQueueWriter<Type> monitor,
        Out<optional<Snapshot>> snapshot) const override {
      if(m_snapshot) {
        *snapshot = *m_snapshot;
      } else {
        *snapshot = none;
      }
      m_queues.push_back(std::move(monitor));
    }

    void monitor(ScopedQueueWriter<Type> monitor) const override {
      auto snapshot = optional<Snapshot>();
      this->monitor(std::move(monitor), out(snapshot));
    }

    void with(const std::function<void ()>& f) const override {
      f();
    }
  };
}

TEST_SUITE("SnapshotPublisher") {
  TEST_CASE("monitor_receives_current_snapshot_when_present") {
    auto publisher = TestSnapshotPublisher();
    publisher.m_snapshot = "current";
    auto queue = std::make_shared<Queue<int>>();
    auto snapshot = optional<std::string>();
    publisher.monitor(queue, out(snapshot));
    REQUIRE(snapshot);
    REQUIRE(*snapshot == "current");
    publisher.publish(10);
    REQUIRE(queue->pop() == 10);
  }

  TEST_CASE("monitor_receives_none_when_no_snapshot") {
    auto publisher = TestSnapshotPublisher();
    auto queue = std::make_shared<Queue<int>>();
    auto snapshot = optional<std::string>();
    publisher.monitor(queue, out(snapshot));
    REQUIRE(!snapshot);
    publisher.publish(7);
    REQUIRE(queue->pop() == 7);
  }

  TEST_CASE("new_monitor_after_updates_gets_latest_snapshot") {
    auto publisher = TestSnapshotPublisher();
    auto queue1 = std::make_shared<Queue<int>>();
    auto snapshot1 = optional<std::string>();
    publisher.monitor(queue1, out(snapshot1));
    REQUIRE(!snapshot1);
    publisher.m_snapshot = "first";
    publisher.publish(1);
    REQUIRE(queue1->pop() == 1);
    auto queue2 = std::make_shared<Queue<int>>();
    auto snapshot2 = optional<std::string>();
    publisher.monitor(queue2, out(snapshot2));
    REQUIRE(snapshot2);
    REQUIRE(*snapshot2 == "first");
    publisher.publish(2);
    REQUIRE(queue1->pop() == 2);
    REQUIRE(queue2->pop() == 2);
  }

  TEST_CASE("with_virtual_invocation_calls_underlying_with") {
    auto publisher = TestSnapshotPublisher();
    publisher.m_snapshot = "snap";
    auto observed = std::string();
    auto func = std::function<void (optional<const std::string&>)>(
      [&] (auto s) {
        if(s) {
          observed = *s;
        } else {
          observed = "none";
        }
      });
    static_cast<const SnapshotPublisher<int, std::string>&>(publisher).with(
      func);
    REQUIRE(observed == "snap");
  }
}
