#include <string>
#include <unordered_map>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Collections/SynchronizedMap.hpp"

using namespace Beam;

TEST_SUITE("SynchronizedMap") {
  TEST_CASE("insert_and_find") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto key = std::string("k");
    REQUIRE(map.insert(key, 10));
    auto found = map.find(key);
    REQUIRE(found.has_value());
    REQUIRE(*found == 10);
    REQUIRE(!map.insert(key, 20));
    auto found_after = map.find(key);
    REQUIRE(found_after.has_value());
    REQUIRE(*found_after == 10);
  }

  TEST_CASE("update_overwrites") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto key = std::string("x");
    map.insert(key, 1);
    map.update(key, 42);
    auto value = map.try_load(key);
    REQUIRE(value.has_value());
    REQUIRE(*value == 42);
  }

  TEST_CASE("get_creates_default") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto key = std::string("missing");
    auto& ref = map.get(key);
    REQUIRE(ref == 0);
    ref = 7;
    auto loaded = map.try_load(key);
    REQUIRE(loaded.has_value());
    REQUIRE(*loaded == 7);
  }

  TEST_CASE("get_or_insert_calls_builder_once") {
    auto map = SynchronizedUnorderedMap<std::string, std::vector<int>>();
    auto key = std::string("vec");
    auto called = false;
    auto& value = map.get_or_insert(key, [&] {
      called = true;
      return std::vector{1, 2, 3};
    });
    REQUIRE(called);
    REQUIRE(value.size() == 3);
    called = false;
    auto& value2 = map.get_or_insert(key, [&] {
      called = true;
      return std::vector{4, 5};
    });
    REQUIRE(!called);
    REQUIRE(value2.size() == 3);
  }

  TEST_CASE("test_and_set_emplaces_via_callback") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto key = std::string("t");
    auto& ref = map.test_and_set(key, [&] (auto& m) {
      m.emplace(key, 99);
    });
    REQUIRE(ref == 99);
    auto loaded = map.try_load(key);
    REQUIRE(loaded.has_value());
    REQUIRE(*loaded == 99);
  }

  TEST_CASE("try_load_returns_copy") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    map.insert(std::string("a"), 5);
    auto copy = map.try_load(std::string("a"));
    REQUIRE(copy.has_value());
    REQUIRE(*copy == 5);
    *copy = 6;
    auto original = map.try_load(std::string("a"));
    REQUIRE(original.has_value());
    REQUIRE(*original == 5);
  }

  TEST_CASE("find_nonconst_returns_reference") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto key = std::string("r");
    map.insert(key, 2);
    auto ref = map.find(key);
    REQUIRE(ref.has_value());
    *ref = 20;
    auto loaded = map.try_load(key);
    REQUIRE(loaded.has_value());
    REQUIRE(*loaded == 20);
  }

  TEST_CASE("erase_removes_key") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto key = std::string("e");
    map.insert(key, 3);
    map.erase(key);
    auto found = map.find(key);
    REQUIRE(!found.has_value());
  }

  TEST_CASE("swap_exchanges_contents") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    auto external = std::unordered_map<std::string, int>();
    external.emplace(std::string("one"), 1);
    map.swap(external);
    REQUIRE(external.find("one") == external.end());
    auto v = map.try_load(std::string("one"));
    REQUIRE(v.has_value());
    REQUIRE(*v == 1);
  }

  TEST_CASE("with_allows_locked_access") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    map.insert(std::string("a"), 1);
    auto size = map.with([] (auto& m) {
      return m.size();
    });
    REQUIRE(size == 1);
    map.with([] (auto& m) {
      m.emplace(std::string("b"), 2);
      return 0;
    });
    auto b = map.try_load(std::string("b"));
    REQUIRE(b.has_value());
    REQUIRE(*b == 2);
  }

  TEST_CASE("clear_removes_all") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    map.insert(std::string("k1"), 1);
    map.insert(std::string("k2"), 2);
    map.clear();
    REQUIRE(!map.find(std::string("k1")).has_value());
    REQUIRE(!map.find(std::string("k2")).has_value());
  }

  TEST_CASE("copy_and_move_constructors") {
    auto source = SynchronizedUnorderedMap<std::string, int>();
    source.insert(std::string("c"), 11);
    auto copy = source;
    auto from_copy = copy.try_load(std::string("c"));
    REQUIRE(from_copy.has_value());
    REQUIRE(*from_copy == 11);
    auto moved = SynchronizedUnorderedMap<std::string, int>(std::move(source));
    auto from_moved = moved.try_load(std::string("c"));
    REQUIRE(from_moved.has_value());
    REQUIRE(*from_moved == 11);
  }

  TEST_CASE("for_each_iterates_entries") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    map.insert(std::string("a"), 1);
    map.insert(std::string("b"), 2);
    map.insert(std::string("c"), 3);
    auto sum = 0;
    auto count = 0;
    map.for_each([&](const auto& entry) {
      sum += entry.second;
      count++;
    });
    REQUIRE(count == 3);
    REQUIRE(sum == 6);
    auto const& const_map = map;
    auto const_sum = 0;
    const_map.for_each([&](const auto& entry) {
      const_sum += entry.second;
    });
    REQUIRE(const_sum == 6);
  }

  TEST_CASE("for_each_value_iterates_values") {
    auto map = SynchronizedUnorderedMap<std::string, int>();
    map.insert(std::string("x"), 10);
    map.insert(std::string("y"), 20);
    map.insert(std::string("z"), 30);
    auto total = 0;
    map.for_each_value([&](const auto& value) {
      total += value;
    });
    REQUIRE(total == 60);
    map.for_each_value([](auto& value) {
      value *= 2;
    });
    auto doubled_total = 0;
    map.for_each_value([&](const auto& value) {
      doubled_total += value;
    });
    REQUIRE(doubled_total == 120);
  }
}
