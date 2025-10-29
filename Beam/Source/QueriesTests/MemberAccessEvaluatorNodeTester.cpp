#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/MemberAccessEvaluatorNode.hpp"

using namespace Beam;

namespace {
  struct Point {
    int x;
    double y;
  };
}

TEST_SUITE("MemberAccessEvaluatorNode") {
  TEST_CASE("constructor") {
    auto point_x_evaluator =
      std::make_unique<ConstantEvaluatorNode<Point>>(Point(589, 3.14));
    auto access_point_x =
      MemberAccessEvaluatorNode(std::move(point_x_evaluator), &Point::x);
    REQUIRE(access_point_x.eval() == 589);
    auto point_y_evaluator =
      std::make_unique<ConstantEvaluatorNode<Point>>(Point(589, 3.14));
    auto access_point_y = MemberAccessEvaluatorNode<Point, double>(
      std::move(point_y_evaluator), &Point::y);
    REQUIRE(access_point_y.eval() == 3.14);
  }
}
