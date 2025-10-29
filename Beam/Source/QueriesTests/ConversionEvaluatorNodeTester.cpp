#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/ConversionEvaluatorNode.hpp"

using namespace Beam;

TEST_SUITE("ConversionEvaluatorNode") {
  TEST_CASE("cast_int_to_double") {
    auto value = std::make_unique<ConstantEvaluatorNode<int>>(123);
    auto node = make_cast_evaluator_node<int, double>(std::move(value));
    REQUIRE(node->eval() == 123.0);
  }

  TEST_CASE("convert_char_array_to_string") {
    auto value =
      std::make_unique<ConstantEvaluatorNode<const char*>>("hello world");
    auto node =
      make_construct_evaluator_node<const char*, std::string>(std::move(value));
    REQUIRE(node->eval() == std::string("hello world"));
  }
}
