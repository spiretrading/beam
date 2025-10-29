#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ReadEvaluatorNode.hpp"

using namespace Beam;

TEST_SUITE("ReadEvaluatorNode") {
  TEST_CASE("read_int") {
    auto value = 42;
    auto node = ReadEvaluatorNode(&value);
    REQUIRE(node.eval() == 42);
  }

  TEST_CASE("read_after_update") {
    auto value = 1;
    auto node = ReadEvaluatorNode(&value);
    REQUIRE(node.eval() == 1);
    value = 7;
    REQUIRE(node.eval() == 7);
  }

  TEST_CASE("read_string") {
    auto value = std::string("hello");
    auto node = ReadEvaluatorNode(&value);
    REQUIRE(node.eval() == "hello");
    value = "world";
    REQUIRE(node.eval() == "world");
  }
}
