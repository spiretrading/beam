#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/FunctionEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("FunctionEvaluatorNode") {
  TEST_CASE("empty_function") {
    struct Function {
      int operator ()() const {
        return 123;
      }
    };
    auto function = FunctionEvaluatorNode(Function());
    REQUIRE(function.Eval() == 123);
  }

  TEST_CASE("unary_function") {
    struct Function {
      std::string operator ()(std::string value) const {
        REQUIRE(value == "hello ");
        return value + "world";
      }
    };
    auto function = FunctionEvaluatorNode(Function(),
      std::make_unique<ConstantEvaluatorNode<std::string>>(
      ConstantEvaluatorNode<std::string>("hello ")));
    REQUIRE(function.Eval() == "hello world");
  }

  TEST_CASE("binary_function") {
    struct Function {
      double operator ()(int lhs, double rhs) const {
        REQUIRE(lhs == 2);
        REQUIRE(rhs == 3.14);
        return lhs * rhs;
      }
    };
    auto function = FunctionEvaluatorNode(Function(),
      std::make_unique<ConstantEvaluatorNode<int>>(
      ConstantEvaluatorNode(2)),
      std::make_unique<ConstantEvaluatorNode<double>>(
      ConstantEvaluatorNode(3.14)));
    REQUIRE(function.Eval() == 6.28);
  }
}
