#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/MemberAccessEvaluatorNode.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

using namespace Beam;
using namespace Beam::Queries;

namespace {
  struct Point {
    int x;
    double y;
  };
}

TEST_SUITE("MemberAccessEvaluatorNode") {
  TEST_CASE("constructor") {
    auto pointXEvaluator = std::make_unique<ConstantEvaluatorNode<Point>>(
      Point{589, 3.14});
    auto accessPointX = MemberAccessEvaluatorNode(std::move(pointXEvaluator),
      &Point::x);
    REQUIRE(accessPointX.Eval() == 589);
    auto pointYEvaluator = std::make_unique<ConstantEvaluatorNode<Point>>(
      Point{589, 3.14});
    auto accessPointY = MemberAccessEvaluatorNode<double, Point>(
      std::move(pointYEvaluator), &Point::y);
    REQUIRE(accessPointY.Eval() == 3.14);
  }
}
