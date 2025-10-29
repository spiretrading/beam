#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/FunctionEvaluatorNode.hpp"

using namespace Beam;

TEST_SUITE("FunctionEvaluatorNode") {
  TEST_CASE("empty_function") {
    struct Function {
      int operator ()() const {
        return 123;
      }
    };
    auto function = FunctionEvaluatorNode(Function());
    REQUIRE(function.eval() == 123);
  }

  TEST_CASE("unary_function") {
    struct Function {
      std::string operator ()(const std::string& value) const {
        REQUIRE(value == "hello ");
        return value + "world";
      }
    };
    auto function = FunctionEvaluatorNode(Function(),
      std::make_unique<ConstantEvaluatorNode<std::string>>("hello "));
    REQUIRE(function.eval() == "hello world");
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
      std::make_unique<ConstantEvaluatorNode<int>>(2),
      std::make_unique<ConstantEvaluatorNode<double>>(3.14));
    REQUIRE(function.eval() == 6.28);
  }
}
