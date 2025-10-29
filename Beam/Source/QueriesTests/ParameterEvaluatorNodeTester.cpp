#include <doctest/doctest.h>
#include "Beam/Queries/ParameterEvaluatorNode.hpp"

using namespace Beam;

TEST_SUITE("ParameterEvaluatorNode") {
  TEST_CASE("int") {
    auto parameter = ParameterEvaluatorNode<int>(0);
    auto value = 123;
    auto valuePtr = static_cast<const void*>(&value);
    parameter.set_parameter(&valuePtr);
    REQUIRE(parameter.eval() == 123);
  }
}
