#include <future>
#include <stdexcept>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include <boost/optional/optional.hpp>
#include "Beam/Threading/TaskRunner.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("TaskRunner") {
  TEST_CASE("single_thread_add") {
    auto runner = TaskRunner();
    auto values = std::vector<int>();
    runner.add([&] {
      values.push_back(1);
    });
    runner.add([&] {
      values.push_back(2);
    });
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 1);
    REQUIRE(values[1] == 2);
    REQUIRE(runner.is_empty());
  }

  TEST_CASE("exception_then_task") {
    auto runner = TaskRunner();
    auto promise = std::promise<int>();
    auto future = promise.get_future();
    runner.add([&] {
      throw std::runtime_error("error");
    });
    runner.add([&] {
      promise.set_value(42);
    });
    REQUIRE(future.get() == 42);
    REQUIRE(runner.is_empty());
  }

  TEST_CASE("is_empty") {
    auto runner = TaskRunner();
    auto started = std::promise<void>();
    auto started_future = started.get_future();
    auto done = std::promise<void>();
    auto done_future = done.get_future();
    auto worker = std::thread([&] {
      runner.add([&] {
        started.set_value();
        done_future.wait();
      });
    });
    started_future.wait();
    REQUIRE(!runner.is_empty());
    done.set_value();
    worker.join();
    REQUIRE(runner.is_empty());
  }

  TEST_CASE("destructor") {
    auto runner = optional<TaskRunner>();
    runner.emplace();
    auto started = std::promise<void>();
    auto started_future = started.get_future();
    auto done = std::promise<void>();
    auto done_future = done.get_future();
    auto reset_started = std::promise<void>();
    auto reset_started_future = reset_started.get_future();
    auto reset_finished = std::promise<void>();
    auto reset_finished_future = reset_finished.get_future();
    auto worker = std::thread([&] {
      runner->add([&] {
        started.set_value();
        done_future.wait();
      });
    });
    started_future.wait();
    auto resetter = std::thread([&] {
      reset_started.set_value();
      runner.reset();
      reset_finished.set_value();
    });
    reset_started_future.wait();
    done.set_value();
    reset_finished_future.wait();
    worker.join();
    resetter.join();
  }
}
