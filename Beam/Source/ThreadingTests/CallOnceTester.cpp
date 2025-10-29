#include <atomic>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Threading/CallOnce.hpp"

using namespace Beam;

TEST_SUITE("CallOnce") {
  TEST_CASE("single_thread_calls_once") {
    auto once = CallOnce<std::mutex>();
    auto counter = std::atomic_int(0);
    auto result1 = once.call([&] {
      counter.fetch_add(1);
    });
    REQUIRE(result1);
    REQUIRE(counter.load() == 1);
    auto result2 = once.call([&] {
      counter.fetch_add(1);
    });
    REQUIRE(!result2);
    REQUIRE(counter.load() == 1);
  }

  TEST_CASE("multiple_threads_call_once") {
    auto once = CallOnce<std::mutex>();
    auto counter = std::atomic_int(0);
    auto thread_count = std::size_t(16);
    auto results = std::vector<int>(thread_count);
    auto threads = std::vector<std::thread>();
    for(auto i = std::size_t(0); i < thread_count; ++i) {
      threads.emplace_back([&, i] {
        auto result = once.call([&] {
          counter.fetch_add(1);
        });
        results[i] = result ? 1 : 0;
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    auto total = 0;
    for(auto i = std::size_t(0); i < thread_count; ++i) {
      total += results[i];
    }
    REQUIRE(total == 1);
    REQUIRE(counter.load() == 1);
  }

  TEST_CASE("exception_does_not_initialize") {
    auto once = CallOnce<std::mutex>();
    auto counter = std::atomic_int(0);
    REQUIRE_THROWS_AS(once.call([&] {
      throw std::runtime_error("failed");
    }), std::runtime_error);
    auto result = once.call([&] {
      counter.fetch_add(1);
    });
    REQUIRE(result);
    REQUIRE(counter.load() == 1);
  }
}
