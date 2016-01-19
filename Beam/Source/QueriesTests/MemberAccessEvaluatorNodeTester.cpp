#include "Beam/QueriesTests/MemberAccessEvaluatorNodeTester.hpp"
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/MemberAccessEvaluatorNode.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

namespace {
  struct Point {
    int x;
    double y;

    Point(int x, double y) : x(x), y(y) {}
  };
}

void MemberAccessEvaluatorNodeTester::TestConstructor() {
  auto pointXEvaluator = std::make_unique<ConstantEvaluatorNode<Point>>(
    MakeConstantEvaluatorNode(Point(589, 3.14)));
  MemberAccessEvaluatorNode<int, Point> accessPointX(std::move(pointXEvaluator),
    &Point::x);
  CPPUNIT_ASSERT(accessPointX.Eval() == 589);
  auto pointYEvaluator = std::make_unique<ConstantEvaluatorNode<Point>>(
    MakeConstantEvaluatorNode(Point(589, 3.14)));
  MemberAccessEvaluatorNode<double, Point> accessPointY(
    std::move(pointYEvaluator), &Point::y);
  CPPUNIT_ASSERT(accessPointY.Eval() == 3.14);
}
