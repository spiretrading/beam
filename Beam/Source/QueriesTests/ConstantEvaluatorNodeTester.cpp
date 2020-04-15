#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ConstantEvaluatorNode") {
  TEST_CASE("int") {
    auto constant = ConstantEvaluatorNode(123);
    REQUIRE(constant.Eval() == 123);
  }

  TEST_CASE("decimal") {
    auto constant = ConstantEvaluatorNode(3.14);
    REQUIRE(constant.Eval() == 3.14);
  }

  TEST_CASE("string") {
    auto constant = ConstantEvaluatorNode(std::string("hello world"));
    REQUIRE(constant.Eval() == "hello world");
  }
}
