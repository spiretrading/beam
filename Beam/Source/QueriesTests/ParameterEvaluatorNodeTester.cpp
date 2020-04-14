#include <doctest/doctest.h>
#include "Beam/Queries/ParameterEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ParameterEvaluatorNode") {
  TEST_CASE("int") {
    auto parameter = ParameterEvaluatorNode<int>(0);
    auto value = 123;
    auto valuePtr = static_cast<const void*>(&value);
    parameter.SetParameter(&valuePtr);
    REQUIRE(parameter.Eval() == 123);
  }
}
