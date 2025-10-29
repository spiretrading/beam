#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"

using namespace Beam;

TEST_SUITE("ConstantEvaluatorNode") {
  TEST_CASE("int") {
    auto constant = ConstantEvaluatorNode(123);
    REQUIRE(constant.eval() == 123);
  }

  TEST_CASE("decimal") {
    auto constant = ConstantEvaluatorNode(3.14);
    REQUIRE(constant.eval() == 3.14);
  }

  TEST_CASE("string") {
    auto constant = ConstantEvaluatorNode(std::string("hello world"));
    REQUIRE(constant.eval() == "hello world");
  }
}
