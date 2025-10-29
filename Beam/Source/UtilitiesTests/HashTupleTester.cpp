#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <doctest/doctest.h>
#include "Beam/Utilities/HashTuple.hpp"

TEST_SUITE("HashTuple") {
  TEST_CASE("hash_tuple_single_element_same_value") {
    auto tuple1 = std::tuple(42);
    auto tuple2 = std::tuple(42);
    auto hasher = std::hash<std::tuple<int>>();
    REQUIRE(hasher(tuple1) == hasher(tuple2));
  }

  TEST_CASE("hash_tuple_single_element_different_values") {
    auto tuple1 = std::tuple(42);
    auto tuple2 = std::tuple(43);
    auto hasher = std::hash<std::tuple<int>>();
    REQUIRE(hasher(tuple1) != hasher(tuple2));
  }

  TEST_CASE("hash_tuple_two_elements_same_value") {
    auto tuple1 = std::tuple(42, 3.14);
    auto tuple2 = std::tuple(42, 3.14);
    auto hasher = std::hash<std::tuple<int, double>>();
    REQUIRE(hasher(tuple1) == hasher(tuple2));
  }

  TEST_CASE("hash_tuple_two_elements_different_first") {
    auto tuple1 = std::tuple(42, 3.14);
    auto tuple2 = std::tuple(43, 3.14);
    auto hasher = std::hash<std::tuple<int, double>>();
    REQUIRE(hasher(tuple1) != hasher(tuple2));
  }

  TEST_CASE("hash_tuple_two_elements_different_second") {
    auto tuple1 = std::tuple(42, 3.14);
    auto tuple2 = std::tuple(42, 2.71);
    auto hasher = std::hash<std::tuple<int, double>>();
    REQUIRE(hasher(tuple1) != hasher(tuple2));
  }

  TEST_CASE("hash_tuple_three_elements_same_value") {
    auto tuple1 = std::tuple(42, 3.14, std::string("test"));
    auto tuple2 = std::tuple(42, 3.14, std::string("test"));
    auto hasher = std::hash<std::tuple<int, double, std::string>>();
    REQUIRE(hasher(tuple1) == hasher(tuple2));
  }

  TEST_CASE("hash_tuple_three_elements_different_values") {
    auto tuple1 = std::tuple(42, 3.14, std::string("test"));
    auto tuple2 = std::tuple(42, 3.14, std::string("other"));
    auto hasher = std::hash<std::tuple<int, double, std::string>>();
    REQUIRE(hasher(tuple1) != hasher(tuple2));
  }

  TEST_CASE("hash_tuple_mixed_types") {
    auto tuple = std::tuple(100, std::string("hello"), 2.5, true);
    auto hasher = std::hash<std::tuple<int, std::string, double, bool>>();
    auto hash1 = hasher(tuple);
    auto hash2 = hasher(tuple);
    REQUIRE(hash1 == hash2);
  }

  TEST_CASE("hash_tuple_in_unordered_set") {
    auto set = std::unordered_set<std::tuple<int, std::string>>();
    auto tuple1 = std::tuple(1, std::string("first"));
    auto tuple2 = std::tuple(2, std::string("second"));
    set.insert(tuple1);
    set.insert(tuple2);
    set.insert(tuple1);
    REQUIRE(set.size() == 2);
    REQUIRE(set.count(tuple1) == 1);
    REQUIRE(set.count(tuple2) == 1);
  }

  TEST_CASE("hash_tuple_in_unordered_map") {
    auto map = std::unordered_map<std::tuple<int, std::string>, double>();
    auto key1 = std::tuple(1, std::string("first"));
    auto key2 = std::tuple(2, std::string("second"));
    map[key1] = 1.5;
    map[key2] = 2.5;
    REQUIRE(map[key1] == 1.5);
    REQUIRE(map[key2] == 2.5);
    REQUIRE(map.size() == 2);
  }

  TEST_CASE("hash_tuple_consistency") {
    auto tuple = std::tuple(42, std::string("test"), 3.14);
    auto hasher = std::hash<std::tuple<int, std::string, double>>();
    auto hash1 = hasher(tuple);
    auto hash2 = hasher(tuple);
    auto hash3 = hasher(tuple);
    REQUIRE(hash1 == hash2);
    REQUIRE(hash2 == hash3);
  }

  TEST_CASE("hash_tuple_order_matters") {
    auto tuple1 = std::tuple(1, 2);
    auto tuple2 = std::tuple(2, 1);
    auto hasher = std::hash<std::tuple<int, int>>();
    REQUIRE(hasher(tuple1) != hasher(tuple2));
  }

  TEST_CASE("hash_tuple_nested_tuples") {
    auto inner1 = std::tuple(1, 2);
    auto inner2 = std::tuple(1, 2);
    auto tuple1 = std::tuple(inner1, 3);
    auto tuple2 = std::tuple(inner2, 3);
    auto hasher = std::hash<std::tuple<std::tuple<int, int>, int>>();
    REQUIRE(hasher(tuple1) == hasher(tuple2));
  }

  TEST_CASE("hash_tuple_nested_tuples_different") {
    auto inner1 = std::tuple(1, 2);
    auto inner2 = std::tuple(1, 3);
    auto tuple1 = std::tuple(inner1, 3);
    auto tuple2 = std::tuple(inner2, 3);
    auto hasher = std::hash<std::tuple<std::tuple<int, int>, int>>();
    REQUIRE(hasher(tuple1) != hasher(tuple2));
  }

  TEST_CASE("hash_tuple_with_zero_values") {
    auto tuple1 = std::tuple(0, 0, 0);
    auto tuple2 = std::tuple(0, 0, 0);
    auto hasher = std::hash<std::tuple<int, int, int>>();
    REQUIRE(hasher(tuple1) == hasher(tuple2));
  }

  TEST_CASE("hash_tuple_many_elements") {
    auto tuple1 = std::tuple(1, 2, 3, 4, 5, 6, 7, 8);
    auto tuple2 = std::tuple(1, 2, 3, 4, 5, 6, 7, 8);
    auto hasher =
      std::hash<std::tuple<int, int, int, int, int, int, int, int>>();
    REQUIRE(hasher(tuple1) == hasher(tuple2));
  }

  TEST_CASE("hash_tuple_erase_from_set") {
    auto set = std::unordered_set<std::tuple<int, int>>();
    auto tuple1 = std::tuple(1, 2);
    auto tuple2 = std::tuple(3, 4);
    set.insert(tuple1);
    set.insert(tuple2);
    REQUIRE(set.size() == 2);
    set.erase(tuple1);
    REQUIRE(set.size() == 1);
    REQUIRE(set.count(tuple1) == 0);
    REQUIRE(set.count(tuple2) == 1);
  }

  TEST_CASE("hash_tuple_update_map_value") {
    auto map = std::unordered_map<std::tuple<int, std::string>, int>();
    auto key = std::tuple(1, std::string("test"));
    map[key] = 100;
    REQUIRE(map[key] == 100);
    map[key] = 200;
    REQUIRE(map[key] == 200);
    REQUIRE(map.size() == 1);
  }
}
