module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>

module Beam;

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
