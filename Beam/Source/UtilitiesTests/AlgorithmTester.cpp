#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Utilities/Algorithm.hpp"

using namespace Beam;

TEST_SUITE("Algorithm") {
  TEST_CASE("find_if_non_managed") {
    auto values = std::vector{1, 2, 3, 4};
    auto result = find_if(values, [] (auto v) { return v == 3; });
    REQUIRE(result.has_value());
    REQUIRE(*result == 3);
  }

  TEST_CASE("find_if_non_managed_not_found") {
    auto values = std::vector{1, 2, 3};
    auto result = find_if(values, [] (auto v) { return v == 42; });
    REQUIRE(!result.has_value());
  }

  TEST_CASE("find_if_managed_pointer") {
    auto a = std::make_shared<int>(10);
    auto b = std::make_shared<int>(20);
    auto c = std::make_shared<int>(30);
    auto range = std::vector{a, b, c};
    auto result = find_if(range, [] (int v) { return v == 20; });
    REQUIRE(result);
    REQUIRE(*result == 20);
  }

  TEST_CASE("remove_first_found") {
    auto container = std::vector{1, 2, 3, 2, 4};
    REQUIRE(remove_first(container, 2));
    REQUIRE(container.size() == 4);
    REQUIRE(std::count(container.begin(), container.end(), 2) == 1);
  }

  TEST_CASE("remove_first_not_found") {
    auto container = std::vector{1, 3, 5};
    REQUIRE(!remove_first(container, 2));
    REQUIRE(container.size() == 3);
  }

  TEST_CASE("swap_erase_orderless") {
    auto container = std::vector{10, 20, 30, 40};
    auto i = container.begin();
    std::advance(i, 1);
    auto result = swap_erase(container, i);
    REQUIRE(container.size() == 3);
    REQUIRE(*result == container[1]);
  }

  TEST_CASE("lookup_const") {
    auto map = std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}};
    const auto& const_map = map;
    auto result = lookup(const_map, "b");
    REQUIRE(result.has_value());
    REQUIRE(*result == 2);
  }

  TEST_CASE("lookup_nonconst") {
    auto map = std::unordered_map<std::string, int>{{"x", 9}};
    auto result = lookup(map, "x");
    REQUIRE(result.has_value());
    REQUIRE(*result == 9);
    *result = 10;
    REQUIRE(map["x"] == 10);
  }

  TEST_CASE("get_or_insert_with_factory") {
    auto map = std::unordered_map<std::string, std::vector<int>>{};
    auto& value = get_or_insert(map, "k", [] { return std::vector{1, 2, 3}; });
    REQUIRE(value.size() == 3);
    REQUIRE(map.find("k") != map.end());
  }

  TEST_CASE("get_or_insert_default_construct") {
    auto map = std::unordered_map<std::string, int>{};
    auto& value = get_or_insert(map, "z");
    REQUIRE(map.find("z") != map.end());
  }
}
