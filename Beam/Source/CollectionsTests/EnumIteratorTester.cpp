#include <doctest/doctest.h>
#include <vector>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Collections/EnumIterator.hpp"

using namespace Beam;

namespace {
  BEAM_ENUM(Color,
    RED,
    GREEN,
    BLUE)
}

TEST_SUITE("EnumIterator") {
  TEST_CASE("iterate_entire_enum") {
    auto values = std::vector<Color::Type>();
    for(auto value : make_range<Color>()) {
      values.push_back(static_cast<Color::Type>(value));
    }
    REQUIRE(values.size() == Color::COUNT);
    REQUIRE(values[0] == Color::Type::RED);
    REQUIRE(values[1] == Color::Type::GREEN);
    REQUIRE(values[2] == Color::Type::BLUE);
  }

  TEST_CASE("begin_and_end") {
    auto range = make_range<Color>();
    auto iterator = begin(range);
    auto end_iterator = end(range);
    REQUIRE(static_cast<Color::Type>(*iterator) == Color::Type::RED);
    auto count = std::size_t(0);
    for(; iterator != end_iterator; ++iterator) {
      ++count;
    }
    REQUIRE(count == Color::COUNT);
  }

  TEST_CASE("increment_semantics") {
    auto range = make_range<Color>();
    auto iterator = begin(range);
    auto prior = iterator;
    iterator++;
    REQUIRE(static_cast<Color::Type>(*prior) == Color::Type::RED);
    REQUIRE(static_cast<Color::Type>(*iterator) == Color::Type::GREEN);
    iterator = begin(range);
    auto result = ++iterator;
    REQUIRE(static_cast<Color::Type>(*iterator) == Color::Type::GREEN);
    REQUIRE(static_cast<Color::Type>(*result) == Color::Type::GREEN);
  }
}
