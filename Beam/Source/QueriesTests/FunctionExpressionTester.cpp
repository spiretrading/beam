#include "Beam/QueriesTests/FunctionExpressionTester.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/FunctionExpression.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void FunctionExpressionTester::TestEmptyFunction() {
  vector<Expression> parameters;
  FunctionExpression function("empty", DecimalType(), parameters);
  CPPUNIT_ASSERT(function.GetName() == "empty");
  CPPUNIT_ASSERT(function.GetType() == DecimalType());
  CPPUNIT_ASSERT(function.GetParameters().empty());
}

void FunctionExpressionTester::TestUnaryFunction() {
  vector<Expression> parameters;
  parameters.push_back(MakeConstantExpression<string>("hello world"));
  FunctionExpression function("unary", BoolType(), parameters);
  CPPUNIT_ASSERT(function.GetName() == "unary");
  CPPUNIT_ASSERT(function.GetType() == BoolType());
  CPPUNIT_ASSERT(function.GetParameters().size() == 1);
  CPPUNIT_ASSERT(typeid(*function.GetParameters()[0]) ==
    typeid(ConstantExpression));
  ConstantExpression p1 =
    function.GetParameters()[0].StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(p1.GetValue()->GetValue<string>() == "hello world");
}

void FunctionExpressionTester::TestBinaryFunction() {
  vector<Expression> parameters;
  parameters.push_back(MakeConstantExpression(5));
  parameters.push_back(MakeConstantExpression(6));
  FunctionExpression function("binary", IntType(), parameters);
  CPPUNIT_ASSERT(function.GetName() == "binary");
  CPPUNIT_ASSERT(function.GetType() == IntType());
  CPPUNIT_ASSERT(function.GetParameters().size() == 2);
  CPPUNIT_ASSERT(typeid(*function.GetParameters()[0]) ==
    typeid(ConstantExpression));
  ConstantExpression p1 =
    function.GetParameters()[0].StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(p1.GetValue()->GetValue<int>() == 5);
  CPPUNIT_ASSERT(typeid(*function.GetParameters()[1]) ==
    typeid(ConstantExpression));
  ConstantExpression p2 =
    function.GetParameters()[1].StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(p2.GetValue()->GetValue<int>() == 6);
}
