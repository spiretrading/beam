#include <doctest/doctest.h>
#include "Beam/IO/ValueSpan.hpp"

using namespace Beam;

struct Small {
  char data[3];
};

TEST_SUITE("ValueSpan") {
  TEST_CASE("construct_and_initial_state") {
    auto value = Small();
    value.data[0] = 'a';
    value.data[1] = 'b';
    value.data[2] = 'c';
    auto span = ValueSpan(Ref(value));
    REQUIRE(span.get_size() == sizeof(Small));
    REQUIRE(span.get_data());
    REQUIRE(span.get_mutable_data());
  }

  TEST_CASE("equality_with_string") {
    auto value = Small();
    value.data[0] = 'a';
    value.data[1] = 'b';
    value.data[2] = 'c';
    auto span = ValueSpan(Ref(value));
    REQUIRE(span == "abc");
  }

  TEST_CASE("write_updates_object") {
    auto value = Small();
    value.data[0] = 'a';
    value.data[1] = 'b';
    value.data[2] = 'c';
    auto span = ValueSpan(Ref(value));
    span.write(1, "Z", 1);
    REQUIRE(value.data[1] == 'Z');
    REQUIRE(span == "aZc");
  }

  TEST_CASE("get_mutable_data_modifies_object") {
    auto value = Small();
    value.data[0] = 'a';
    value.data[1] = 'b';
    value.data[2] = 'c';
    auto span = ValueSpan(Ref(value));
    auto ptr = span.get_mutable_data();
    ptr[0] = 'X';
    REQUIRE(value.data[0] == 'X');
    REQUIRE(span == "Xbc");
  }

  TEST_CASE("grow_and_shrink_behavior") {
    auto value = Small();
    value.data[0] = 'a';
    value.data[1] = 'b';
    value.data[2] = 'c';
    auto span = ValueSpan(Ref(value));
    auto before = span.get_size();
    REQUIRE(before == sizeof(Small));
    auto shrunk = span.shrink(1);
    REQUIRE(shrunk == 1);
    REQUIRE(span.get_size() == sizeof(Small) - 1);
    auto shrunk_all = span.shrink(5);
    REQUIRE(shrunk_all == sizeof(Small) - 1);
    REQUIRE(span.get_size() == 0);
    auto grown = span.grow(2);
    REQUIRE(grown == 2);
    REQUIRE(span.get_size() == 2);
    auto grown_more = span.grow(5);
    REQUIRE(grown_more == (sizeof(Small) - 2));
    REQUIRE(span.get_size() == sizeof(Small));
  }

  TEST_CASE("write_out_of_range_throws") {
    auto value = Small();
    value.data[0] = 'a';
    value.data[1] = 'b';
    value.data[2] = 'c';
    auto span = ValueSpan(Ref(value));
    REQUIRE_THROWS_AS(span.write(2, "AB", 2), std::out_of_range);
  }
}
