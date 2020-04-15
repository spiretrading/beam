#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/IndexedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("IndexedQuery") {
  TEST_CASE("default_constructor") {
    auto query = IndexedQuery<int>();
    REQUIRE(query.GetIndex() == 0);
  }

  TEST_CASE("index_constructor") {
    auto intIndex = IndexedQuery<int>(123);
    REQUIRE(intIndex.GetIndex() == 123);
    auto stringIndex = IndexedQuery<std::string>("hello world");
    REQUIRE(stringIndex.GetIndex() == "hello world");
  }

  TEST_CASE("set_index") {
    auto intIndex = IndexedQuery<int>();
    REQUIRE(intIndex.GetIndex() != 123);
    intIndex.SetIndex(123);
    REQUIRE(intIndex.GetIndex() == 123);
    auto stringIndex = IndexedQuery<std::string>();
    REQUIRE(stringIndex.GetIndex() != "hello world");
    stringIndex.SetIndex("hello world");
    REQUIRE(stringIndex.GetIndex() == "hello world");
  }
}
