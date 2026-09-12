#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Threading/Sync.hpp"

using namespace Beam;

namespace {
  template<typename T, typename U>
  concept HasExchange = requires(T& value) {
    { value.exchange(std::declval<U>()) } -> std::same_as<typename T::Value>;
  };
}

TEST_SUITE("Sync") {
  TEST_CASE("construct_and_load") {
    auto sync = Sync(42);
    REQUIRE(sync.load() == 42);
  }

  TEST_CASE("assign_value") {
    auto sync = Sync(0);
    sync = 99;
    REQUIRE(sync.load() == 99);
  }

  TEST_CASE("exchange") {
    auto sync = Sync(42);
    REQUIRE(sync.exchange(99) == 42);
    REQUIRE(sync.load() == 99);
  }

  TEST_CASE("exchange_ownership") {
    auto sync = Sync(std::make_unique<int>(42));
    auto previous = sync.exchange(std::make_unique<int>(99));
    REQUIRE(previous);
    REQUIRE(*previous == 42);
    previous = sync.exchange(nullptr);
    REQUIRE(previous);
    REQUIRE(*previous == 99);
    REQUIRE(sync.with([&] (const auto& value) { return !value; }));
  }

  TEST_CASE("exchange_constraints") {
    REQUIRE(HasExchange<Sync<int>, int>);
    REQUIRE(HasExchange<Sync<std::shared_ptr<int>>, std::nullptr_t>);
    REQUIRE(HasExchange<Sync<std::unique_ptr<int>>, std::unique_ptr<int>>);
    REQUIRE(!HasExchange<Sync<int>, std::string>);
    REQUIRE(!HasExchange<Sync<const int>, int>);
    REQUIRE(!HasExchange<const Sync<int>, int>);
    REQUIRE(!HasExchange<Sync<std::atomic<int>>, int>);
    REQUIRE(!HasExchange<Sync<std::unique_ptr<int>>, std::unique_ptr<int>&>);
  }

  TEST_CASE("concurrent_exchanges") {
    constexpr auto THREAD_COUNT = 4;
    constexpr auto EXCHANGES_PER_THREAD = 25;
    auto sync = Sync(0);
    auto values = std::vector<int>(THREAD_COUNT * EXCHANGES_PER_THREAD + 1);
    auto threads = std::vector<std::thread>();
    for(auto i = 0; i != THREAD_COUNT; ++i) {
      threads.emplace_back([&] (int index) {
        for(auto j = 0; j != EXCHANGES_PER_THREAD; ++j) {
          auto value = index * EXCHANGES_PER_THREAD + j + 1;
          values[value - 1] = sync.exchange(value);
        }
      }, i);
    }
    for(auto& thread : threads) {
      thread.join();
    }
    values.back() = sync.load();
    std::ranges::sort(values);
    for(auto i = 0; i <= THREAD_COUNT * EXCHANGES_PER_THREAD; ++i) {
      REQUIRE(values[i] == i);
    }
  }

  TEST_CASE("copy_construct_from_sync") {
    auto s1 = Sync(123);
    auto s2 = Sync(s1);
    REQUIRE(s2.load() == 123);
  }

  TEST_CASE("move_construct_from_sync") {
    auto s1 = Sync(std::make_shared<std::string>("hello"));
    auto s2 = Sync(std::move(s1));
    REQUIRE(!s1.load());
    REQUIRE(*s2.load() == "hello");
  }

  TEST_CASE("copy_assign_from_sync") {
    auto s1 = Sync(77);
    auto s2 = Sync(0);
    s2 = s1;
    REQUIRE(s2.load() == 77);
  }

  TEST_CASE("move_assign_from_sync") {
    auto s1 = Sync(std::make_shared<std::string>("abc"));
    auto s2 = Sync(std::make_shared<std::string>("xyz"));
    s2 = std::move(s1);
    REQUIRE(!s1.load());
    REQUIRE(*s2.load() == "abc");
  }

  TEST_CASE("with_mutable_access") {
    auto sync = Sync(5);
    sync.with([&] (auto& value) {
      value += 10;
    });
    REQUIRE(sync.load() == 15);
  }

  TEST_CASE("with_const_access") {
    auto sync = Sync(12);
    auto result = sync.with([&] (auto const& value) {
      return value * 2;
    });
    REQUIRE(result == 24);
  }

  TEST_CASE("with_two_syncs") {
    auto s1 = Sync(2);
    auto s2 = Sync(3);
    auto result = s1.with(s2, [&] (auto& a, auto& b) {
      return a + b;
    });
    REQUIRE(result == 5);
  }

  TEST_CASE("concurrent_updates") {
    auto sync = Sync(0);
    auto threads = std::vector<std::thread>();
    for(auto i = 0; i < 10; ++i) {
      threads.emplace_back([&] {
        for(auto j = 0; j < 100; ++j) {
          sync.with([&] (auto& value) {
            ++value;
          });
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    REQUIRE(sync.load() == 1000);
  }
}
