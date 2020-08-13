#include <vector>
#include <doctest/doctest.h>
#include "Beam/Collections/DereferenceIterator.hpp"
#include "Beam/Collections/IndexIterator.hpp"
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

  TEST_CASE("move") {
    auto source = std::vector<int>();
    source.push_back(10);
    auto view1 = View(source);
    auto view2 = std::move(view1);
    REQUIRE(view1.empty());
    REQUIRE(*view2.begin() == 10);
  }

  TEST_CASE("indexed_view") {
    auto v = std::vector<int>();
    v.push_back(3);
    v.push_back(2);
    v.push_back(7);
    auto view = MakeIndexView(v);
    REQUIRE(view.begin()->GetIndex() == 0);
    REQUIRE(view.begin()->GetValue() == 3);
  }

  TEST_CASE("dereference_view") {
    auto v = std::vector<std::unique_ptr<int>>();
    v.push_back(std::make_unique<int>(5));
    v.push_back(std::make_unique<int>(1));
    v.push_back(std::make_unique<int>(2));
    auto view = MakeDereferenceView(v);
    REQUIRE(*view.begin() == 5);
    REQUIRE(*(view.begin() + 1) == 1);
    REQUIRE(*(view.begin() + 2) == 2);
  }

  TEST_CASE("move_const_view") {
    auto v = std::vector<int>();
    v.push_back(3);
    auto view = View(v);
    auto cview = View<const int>(std::move(view));
    REQUIRE(view.empty());
    REQUIRE(*cview.begin() == 3);
  }

  TEST_CASE("copy_const_view") {
    auto v = std::vector<int>();
    v.push_back(3);
    auto view = View(v);
    auto cview = View<const int>(view);
    REQUIRE(*view.begin() == 3);
    REQUIRE(*cview.begin() == 3);
  }
}
