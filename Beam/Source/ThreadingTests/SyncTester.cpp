#include <string>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Threading/Sync.hpp"

using namespace Beam;

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
