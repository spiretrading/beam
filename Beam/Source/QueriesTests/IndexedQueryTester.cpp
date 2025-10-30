#include <doctest/doctest.h>
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("IndexedQuery") {
  TEST_CASE("default_constructor") {
    auto query = IndexedQuery<int>();
    REQUIRE(query.get_index() == 0);
  }

  TEST_CASE("index_constructor") {
    auto int_index = IndexedQuery(123);
    REQUIRE(int_index.get_index() == 123);
    auto string_index = IndexedQuery(std::string("hello world"));
    REQUIRE(string_index.get_index() == "hello world");
  }

  TEST_CASE("set_index") {
    auto int_index = IndexedQuery<int>();
    REQUIRE(int_index.get_index() != 123);
    int_index.set_index(123);
    REQUIRE(int_index.get_index() == 123);
    auto string_index = IndexedQuery<std::string>();
    REQUIRE(string_index.get_index() != "hello world");
    string_index.set_index("hello world");
    REQUIRE(string_index.get_index() == "hello world");
  }

  TEST_CASE("stream") {
    auto int_index = IndexedQuery(123);
    REQUIRE(to_string(int_index) == "123");
    auto string_index = IndexedQuery(std::string("hello world"));
    REQUIRE(to_string(string_index) == "hello world");
  }
}
