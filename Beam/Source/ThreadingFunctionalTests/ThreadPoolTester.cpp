#include <atomic>
#include <future>
#include <memory>
#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Threading/ThreadPool.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace Beam::Tests {
  struct ThreadPoolTester {
    static std::unique_ptr<ThreadPool> make_pool(std::size_t count) {
      auto pool = std::unique_ptr<ThreadPool>(new ThreadPool());
      pool->m_max_thread_count = count;
      return pool;
    }

    static void notify_idle(ThreadPool& pool) {
      auto lock = std::lock_guard(pool.m_mutex);
      for(auto thread : pool.m_threads) {
        thread->m_task_available_condition.notify_all();
      }
    }

    static void stop_idle(ThreadPool& pool) {
      auto lock = std::lock_guard(pool.m_mutex);
      for(auto thread : pool.m_threads) {
        thread->stop();
      }
    }

    static bool is_closing(ThreadPool& pool) {
      auto lock = std::lock_guard(pool.m_mutex);
      return pool.m_max_thread_count == 0;
    }
  };
}

TEST_SUITE("ThreadPool") {
  TEST_CASE("queue") {
    auto pool = ThreadPoolTester::make_pool(2);
    auto release = std::promise<void>();
    auto ready = release.get_future().share();
    auto results = std::vector<Async<int>>(16);
    for(auto i = 0; i < static_cast<int>(results.size()); ++i) {
      pool->queue([=] {
        ready.wait();
        return i;
      }, results[i].get_eval());
    }
    REQUIRE(pool->get_queued_count() == results.size());
    release.set_value();
    for(auto i = 0; i < static_cast<int>(results.size()); ++i) {
      REQUIRE(results[i].get() == i);
    }
    pool->wait_until_idle();
    REQUIRE(pool->is_idle());
  }

  TEST_CASE("task_exception") {
    auto pool = ThreadPoolTester::make_pool(1);
    auto failure = Async<void>();
    pool->queue([] {
      throw std::runtime_error("task failed");
    }, failure.get_eval());
    REQUIRE_THROWS_AS(failure.get(), std::runtime_error);
    auto success = Async<int>();
    pool->queue([] { return 42; }, success.get_eval());
    REQUIRE(success.get() == 42);
    pool->wait_until_idle();
  }

  TEST_CASE("idle_notification") {
    auto pool = ThreadPoolTester::make_pool(1);
    for(auto i = 0; i < 100; ++i) {
      auto result = Async<int>();
      pool->queue([=] { return i; }, result.get_eval());
      REQUIRE(result.get() == i);
      pool->wait_until_idle();
      ThreadPoolTester::notify_idle(*pool);
      std::this_thread::yield();
    }
  }

  TEST_CASE("shutdown_active_and_idle_workers") {
    auto pool = ThreadPoolTester::make_pool(2);
    auto started = std::promise<void>();
    auto release = std::promise<void>();
    auto ready = release.get_future();
    auto finished = std::atomic_bool(false);
    auto active = Async<void>();
    pool->queue([&] {
      started.set_value();
      ready.wait();
      finished = true;
    }, active.get_eval());
    started.get_future().wait();
    auto idle = Async<int>();
    pool->queue([] { return 42; }, idle.get_eval());
    REQUIRE(idle.get() == 42);
    auto pointer = pool.get();
    auto shutdown = std::async(std::launch::async, [&] {
      pool.reset();
    });
    while(!ThreadPoolTester::is_closing(*pointer)) {
      std::this_thread::yield();
    }
    auto is_waiting = shutdown.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout;
    release.set_value();
    shutdown.get();
    active.get();
    REQUIRE(is_waiting);
    REQUIRE(finished.load());
  }

  TEST_CASE("shutdown_retiring_workers") {
    for(auto i = 0; i < 100; ++i) {
      auto pool = ThreadPoolTester::make_pool(1);
      auto result = Async<int>();
      pool->queue([] { return 42; }, result.get_eval());
      REQUIRE(result.get() == 42);
      pool->wait_until_idle();
      ThreadPoolTester::stop_idle(*pool);
      pool.reset();
    }
  }
}
