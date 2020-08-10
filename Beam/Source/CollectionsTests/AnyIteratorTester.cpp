#include <vector>
#include <doctest/doctest.h>
#include "Beam/Collections/AnyIterator.hpp"

using namespace Beam;

TEST_SUITE("AnyIterator") {
  TEST_CASE("empty") {
    auto i = AnyIterator<int>();
    auto j = AnyIterator<int>();
    REQUIRE(i == j);
  }

  TEST_CASE("move") {
    auto v = std::vector<int>();
    v.push_back(3);
    v.push_back(1);
    v.push_back(4);
    auto i = AnyIterator(v.begin());
    auto j = std::move(i);
    REQUIRE(i == AnyIterator<int>());
    REQUIRE(*j == 3);
  }
}
