#include <doctest/doctest.h>
#include "Beam/IO/Span.hpp"

using namespace Beam;

TEST_SUITE("Span") {
  TEST_CASE("default") {
    auto span = Span();
    REQUIRE(span.get_size() == 0);
    REQUIRE(!span.get_data());
    REQUIRE(!span.get_mutable_data());
  }

  TEST_CASE("pointer_and_size") {
    auto buffer = std::array{'a', 'b', 'c', 'd', 'e', '\0'};
    auto span = Span(buffer.data(), static_cast<std::size_t>(5));
    REQUIRE(span.get_size() == 0);
    REQUIRE(span.get_data() == buffer.data());
    REQUIRE(span.get_mutable_data() == buffer.data());
  }

  TEST_CASE("write") {
    auto buffer = std::array{'0', '1', '2', '3', '4'};
    auto span = Span(buffer.data(), buffer.size());
    auto src = std::array{'X', 'Y'};
    span.write(1, src.data(), 2);
    REQUIRE(buffer[0] == '0');
    REQUIRE(buffer[1] == 'X');
    REQUIRE(buffer[2] == 'Y');
    REQUIRE(buffer[3] == '3');
    REQUIRE(buffer[4] == '4');
  }

  TEST_CASE("write_zero") {
    auto buffer = std::array{'a', 'b', 'c'};
    auto span = Span(buffer.data(), buffer.size());
    span.write(1, nullptr, 0);
    REQUIRE(buffer[0] == 'a');
    REQUIRE(buffer[1] == 'b');
    REQUIRE(buffer[2] == 'c');
  }

  TEST_CASE("write_out_of_range") {
    auto buffer = std::array{'a', 'b', 'c', 'd'};
    auto span = Span(buffer.data(), buffer.size());
    auto src = std::array{'X', 'Y', 'Z'};
    REQUIRE_THROWS_AS(span.write(2, src.data(), 3), std::out_of_range);
    REQUIRE_THROWS_AS(span.write(5, src.data(), 1), std::out_of_range);
  }

  TEST_CASE("grow") {
    auto buffer = std::array{'a', 'b', 'c', 'd'};
    auto span = Span(buffer.data(), buffer.size());
    REQUIRE(span.grow(2 * buffer.size()) == buffer.size());
    REQUIRE(span.grow(1) == 0);
  }

  TEST_CASE("shrink") {
    auto buffer = std::array{'a', 'b', 'c', 'd'};
    auto span = Span(buffer.data(), buffer.size());
    auto before = span.get_size();
    span.shrink(1);
    REQUIRE(span.get_size() == before);
    span.shrink(100);
    REQUIRE(span.get_size() == before);
  }
}
