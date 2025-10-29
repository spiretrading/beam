#include <string>
#include <doctest/doctest.h>
#include "Beam/Utilities/KeyValueCache.hpp"

using namespace Beam;

TEST_SUITE("KeyValueCache") {
  TEST_CASE("constructor") {
    auto cache = KeyValueCache<int, std::string>([] (const auto& key) {
      return std::to_string(key);
    });
    auto& value = cache.load(5);
    REQUIRE(value == "5");
  }

  TEST_CASE("load_cached_value") {
    auto counter = 0;
    auto cache = KeyValueCache<int, std::string>([&](const auto& key) {
      ++counter;
      return std::to_string(key);
    });
    auto& value1 = cache.load(10);
    auto& value2 = cache.load(10);
    REQUIRE(value1 == "10");
    REQUIRE(value2 == "10");
    REQUIRE(counter == 1);
  }

  TEST_CASE("load_multiple_keys") {
    auto cache = KeyValueCache<int, std::string>([] (const auto& key) {
      return std::to_string(key);
    });
    auto& value1 = cache.load(1);
    auto& value2 = cache.load(2);
    auto& value3 = cache.load(3);
    REQUIRE(value1 == "1");
    REQUIRE(value2 == "2");
    REQUIRE(value3 == "3");
  }

  TEST_CASE("load_returns_reference") {
    auto cache = KeyValueCache<int, std::string>([] (const auto& key) {
      return std::string("value");
    });
    auto& value1 = cache.load(1);
    auto& value2 = cache.load(1);
    REQUIRE(&value1 == &value2);
  }

  TEST_CASE("load_with_string_key") {
    auto cache = KeyValueCache<std::string, int>([] (const auto& key) {
      return static_cast<int>(key.length());
    });
    auto& value = cache.load("hello");
    REQUIRE(value == 5);
  }

  TEST_CASE("load_with_string_value") {
    auto cache = KeyValueCache<int, std::string>([] (const auto& key) {
      return "key_" + std::to_string(key);
    });
    auto& value = cache.load(42);
    REQUIRE(value == "key_42");
  }

  TEST_CASE("set_source_after_construction") {
    auto cache = KeyValueCache<int, std::string>();
    cache.set_source([](const auto& key) {
      return std::to_string(key * 2);
    });
    auto& value = cache.load(5);
    REQUIRE(value == "10");
  }

  TEST_CASE("set_source_replaces_existing_source") {
    auto cache = KeyValueCache<int, std::string>([] (const auto& key) {
      return std::to_string(key);
    });
    cache.set_source([](const auto& key) {
      return std::to_string(key * 3);
    });
    auto& value = cache.load(4);
    REQUIRE(value == "12");
  }

  TEST_CASE("set_source_does_not_clear_cache") {
    auto cache = KeyValueCache<int, std::string>([] (const auto& key) {
      return std::to_string(key);
    });
    auto& value1 = cache.load(10);
    cache.set_source([] (const auto& key) {
      return std::to_string(key * 100);
    });
    auto& value2 = cache.load(10);
    REQUIRE(value1 == "10");
    REQUIRE(value2 == "10");
    REQUIRE(&value1 == &value2);
  }

  TEST_CASE("source_call_count") {
    auto counter = 0;
    auto cache = KeyValueCache<int, int>([&] (const auto& key) {
      ++counter;
      return key * key;
    });
    cache.load(2);
    cache.load(3);
    cache.load(2);
    cache.load(3);
    cache.load(4);
    REQUIRE(counter == 3);
  }

  TEST_CASE("load_interleaved_keys") {
    auto cache = KeyValueCache<int, int>([] (const auto& key) {
      return key * 10;
    });
    auto& v1 = cache.load(1);
    auto& v2 = cache.load(2);
    auto& v3 = cache.load(1);
    auto& v4 = cache.load(3);
    auto& v5 = cache.load(2);
    REQUIRE(v1 == 10);
    REQUIRE(v2 == 20);
    REQUIRE(v3 == 10);
    REQUIRE(v4 == 30);
    REQUIRE(v5 == 20);
    REQUIRE(&v1 == &v3);
    REQUIRE(&v2 == &v5);
  }
}
