#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/ConversionEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ConversionEvaluatorNode") {
  TEST_CASE("cast_int_to_double") {
    auto value = std::make_unique<ConstantEvaluatorNode<int>>(123);
    auto node = MakeCastEvaluatorNode<int, double>(std::move(value));
    REQUIRE(node->Eval() == 123.0);
  }

  TEST_CASE("convert_char_array_to_string") {
    auto value = std::make_unique<ConstantEvaluatorNode<const char*>>(
      "hello world");
    auto node = MakeConstructEvaluatorNode<const char*, std::string>(
      std::move(value));
    REQUIRE(node->Eval() == std::string("hello world"));
  }
}
