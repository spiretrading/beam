#include <memory>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Collections/SharedIterator.hpp"

using namespace Beam;

TEST_SUITE("SharedIterator") {
  TEST_CASE("dereference_and_arrow") {
    auto collection =
      std::make_shared<std::vector<int>>(std::vector{10, 20, 30});
    auto iterator = SharedIterator(collection, collection->begin());
    REQUIRE(*iterator == 10);
    REQUIRE(iterator.operator->() != nullptr);
    REQUIRE(*iterator == *(iterator.operator->()));
  }

  TEST_CASE("increment_and_decrement") {
    auto collection = std::make_shared<std::vector<int>>(std::vector{1, 2, 3});
    auto iterator = SharedIterator(collection, collection->begin());
    ++iterator;
    REQUIRE(*iterator == 2);
    --iterator;
    REQUIRE(*iterator == 1);
    iterator += 2;
    REQUIRE(*iterator == 3);
    iterator -= 1;
    REQUIRE(*iterator == 2);
  }

  TEST_CASE("arithmetic_and_distance") {
    auto collection =
      std::make_shared<std::vector<int>>(std::vector{5, 6, 7, 8});
    auto iterator = SharedIterator(collection, collection->begin());
    auto advanced = iterator + 2;
    REQUIRE(*advanced == 7);
    auto distance = advanced - iterator;
    REQUIRE(distance == 2);
  }

  TEST_CASE("equality_and_inequality") {
    auto collection = std::make_shared<std::vector<int>>(std::vector{42, 43});
    auto left = SharedIterator(collection, collection->begin());
    auto right = SharedIterator(collection, collection->begin());
    REQUIRE(left == right);
    ++right;
    REQUIRE(left != right);
  }

  TEST_CASE("shared_ownership_keeps_collection_alive") {
    auto iterator = [&] () {
      auto collection = std::make_shared<std::vector<int>>(std::vector{9, 10});
      return SharedIterator(collection, collection->begin());
    }();
    REQUIRE(*iterator == 9);
    ++iterator;
    REQUIRE(*iterator == 10);
  }
}
