#include <vector>
#include <memory>
#include <doctest/doctest.h>
#include "Beam/Collections/SynchronizedList.hpp"

using namespace Beam;

TEST_SUITE("SynchronizedList") {
  TEST_CASE("push_back_and_load") {
    auto list = SynchronizedVector<int>();
    list.push_back(10);
    list.push_back(20);
    auto snapshot = list.load();
    REQUIRE(snapshot.size() == 2);
    REQUIRE(snapshot[0] == 10);
    REQUIRE(snapshot[1] == 20);
  }

  TEST_CASE("append_and_clear") {
    auto list = SynchronizedVector<int>();
    auto container = std::vector{1, 2, 3};
    list.append(container);
    auto snapshot = list.load();
    REQUIRE(snapshot.size() == 3);
    list.clear();
    REQUIRE(list.load().empty());
  }

  TEST_CASE("erase_and_erase_if") {
    auto list = SynchronizedVector<int>();
    list.append(std::vector{1, 2, 3, 2});
    list.erase(2);
    auto snapshot = list.load();
    REQUIRE(snapshot.size() == 2);
    REQUIRE(snapshot[0] == 1);
    REQUIRE(snapshot[1] == 3);
    list.erase_if([](const auto& value) {
      return value % 2 == 1;
    });
    REQUIRE(list.load().empty());
  }

  TEST_CASE("for_each_and_const_for_each") {
    auto list = SynchronizedVector<int>();
    list.append(std::vector{1, 2, 3});
    auto sum = 0;
    list.for_each([&] (auto& value) {
      sum += value;
    });
    REQUIRE(sum == 6);
    auto& snapshot = list;
    auto const_sum = 0;
    snapshot.for_each([&](const auto& value) {
      const_cast<int&>(const_cast<const int&>(value));
      const_sum += value;
    });
    REQUIRE(const_sum == 6);
  }

  TEST_CASE("copy_and_move_construction") {
    auto source = SynchronizedVector<int>();
    source.append(std::vector{4, 5});
    auto copy = SynchronizedVector<int>(source);
    REQUIRE(copy.load().size() == 2);
    REQUIRE(copy.load()[0] == 4);
    REQUIRE(copy.load()[1] == 5);
    auto moved = SynchronizedVector<int>(std::move(source));
    REQUIRE(moved.load().size() == 2);
    REQUIRE(moved.load()[0] == 4);
    REQUIRE(moved.load()[1] == 5);
  }

  TEST_CASE("swap_with_external_list_and_with_method") {
    auto list = SynchronizedVector<int>();
    list.append(std::vector{7, 8});
    auto external = std::vector{1, 2};
    list.swap(external);
    REQUIRE(external.size() == 2);
    REQUIRE(external[0] == 7);
    REQUIRE(external[1] == 8);
    auto snapshot = list.load();
    REQUIRE(snapshot.size() == 2);
    REQUIRE(snapshot[0] == 1);
    REQUIRE(snapshot[1] == 2);
    auto result = list.with([] (std::vector<int>& container) {
      container.push_back(99);
      return container.size();
    });
    REQUIRE(result == 3);
    REQUIRE(list.load().back() == 99);
  }
}
