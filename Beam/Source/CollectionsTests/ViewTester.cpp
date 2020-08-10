#include <vector>
#include <doctest/doctest.h>
#include "Beam/Collections/IndexedIterator.hpp"
#include "Beam/Collections/View.hpp"

using namespace Beam;

TEST_SUITE("View") {
  TEST_CASE("empty") {
    auto view = View<int>();
    REQUIRE(view.begin() == view.end());
    REQUIRE(view.empty());
    REQUIRE(view.size() == 0);
  }

  TEST_CASE("from_collection") {
    auto v = std::vector<int>();
    v.push_back(3);
    v.push_back(2);
    v.push_back(1);
    auto view = View(v);
    REQUIRE(std::distance(view.begin(), view.end()) == 3);
    REQUIRE(view.size() == 3);
  }

  TEST_CASE("from_iterators") {
    auto v = std::vector<int>();
    v.push_back(3);
    v.push_back(2);
    auto view = View(v.begin(), v.end());
    REQUIRE(std::distance(view.begin(), view.end()) == 2);
    REQUIRE(view.size() == 2);
  }

  TEST_CASE("indexed_view") {
    auto v = std::vector<int>();
    v.push_back(3);
    v.push_back(2);
    v.push_back(7);
    auto view = MakeIndexedView(v);
    REQUIRE(view.begin()->GetIndex() == 0);
    REQUIRE(view.begin()->GetValue() == 3);
  }

  TEST_CASE("move") {
    auto source = std::vector<int>();
    source.push_back(10);
    auto view1 = View(source);
    auto view2 = std::move(view1);
    REQUIRE(view1.empty());
    REQUIRE(*view2.begin() == 10);
  }
}
