#include <string>
#include <unordered_set>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Collections/SynchronizedSet.hpp"

using namespace Beam;

TEST_SUITE("Include/Beam/Collections/SynchronizedSet.hpp") {
  TEST_CASE("insert_and_contains") {
    auto set = SynchronizedUnorderedSet<int>();
    REQUIRE(set.insert(10));
    REQUIRE(set.contains(10));
    REQUIRE(!set.insert(10));
  }

  TEST_CASE("get_inserts_value") {
    auto set = SynchronizedUnorderedSet<std::string>();
    auto value = std::string("hello");
    auto stored = set.get(value);
    REQUIRE(stored == value);
    REQUIRE(set.contains(value));
  }

  TEST_CASE("try_load_returns_copy") {
    auto set = SynchronizedUnorderedSet<int>();
    set.insert(5);
    auto copy = set.try_load(5);
    REQUIRE(copy.has_value());
    REQUIRE(*copy == 5);
    *copy = 6;
    auto original = set.try_load(5);
    REQUIRE(original.has_value());
    REQUIRE(*original == 5);
  }

  TEST_CASE("test_and_set_calls_callback_only_when_missing") {
    auto set = SynchronizedUnorderedSet<int>();
    auto called = false;
    set.test_and_set(7, [&] { called = true; });
    REQUIRE(called);
    called = false;
    set.test_and_set(7, [&] { called = true; });
    REQUIRE(!called);
  }

  TEST_CASE("find_returns_presence") {
    auto set = SynchronizedUnorderedSet<std::string>();
    auto key = std::string("k");
    REQUIRE(!set.find(key).has_value());
    set.insert(key);
    auto found = set.find(key);
    REQUIRE(found.has_value());
    REQUIRE(*found == key);
  }

  TEST_CASE("update_ensures_presence") {
    auto set = SynchronizedUnorderedSet<int>();
    set.update(3);
    REQUIRE(set.contains(3));
    set.update(3);
    REQUIRE(set.contains(3));
  }

  TEST_CASE("erase_removes_value") {
    auto set = SynchronizedUnorderedSet<int>();
    set.insert(2);
    REQUIRE(set.contains(2));
    set.erase(2);
    REQUIRE(!set.contains(2));
  }

  TEST_CASE("swap_exchanges_contents") {
    auto set = SynchronizedUnorderedSet<int>();
    auto external = std::unordered_set<int>();
    external.insert(1);
    set.swap(external);
    REQUIRE(external.empty());
    REQUIRE(set.contains(1));
  }

  TEST_CASE("with_allows_locked_access") {
    auto set = SynchronizedUnorderedSet<int>();
    set.insert(8);
    auto size = set.with([] (auto& s) {
      return s.size();
    });
    REQUIRE(size == 1);
    set.with([] (auto& s) {
      s.insert(9);
      return 0;
    });
    REQUIRE(set.contains(9));
  }

  TEST_CASE("clear_removes_all") {
    auto set = SynchronizedUnorderedSet<int>();
    set.insert(4);
    set.insert(5);
    set.clear();
    REQUIRE(!set.contains(4));
    REQUIRE(!set.contains(5));
  }

  TEST_CASE("copy_and_move_constructors") {
    auto source = SynchronizedUnorderedSet<int>();
    source.insert(11);
    auto copy = source;
    auto from_copy = copy.try_load(11);
    REQUIRE(from_copy.has_value());
    REQUIRE(*from_copy == 11);
    auto moved = SynchronizedUnorderedSet<int>(std::move(source));
    auto from_moved = moved.try_load(11);
    REQUIRE(from_moved.has_value());
    REQUIRE(*from_moved == 11);
  }
}
