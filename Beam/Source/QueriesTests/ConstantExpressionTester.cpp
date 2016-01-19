#include "Beam/QueriesTests/ConstantExpressionTester.hpp"
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/NativeDataType.hpp"
#include "Beam/Queries/NativeValue.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ConstantExpressionTester::TestMakeConstantExpression() {
  ConstantExpression intExpression = MakeConstantExpression(123);
  CPPUNIT_ASSERT(intExpression.GetValue()->GetValue<int>() == 123);
  ConstantExpression stringExpression = MakeConstantExpression<string>(
    "hello world");
  CPPUNIT_ASSERT(stringExpression.GetValue()->GetValue<string>() ==
    "hello world");
}

void ConstantExpressionTester::TestInt() {
  ConstantExpression constantExpression(MakeNativeValue(123));
  CPPUNIT_ASSERT(constantExpression.GetType() == NativeDataType<int>());
  CPPUNIT_ASSERT(constantExpression.GetValue() == MakeNativeValue(123));
  Expression expression = constantExpression;
  CPPUNIT_ASSERT(expression->GetType() == NativeDataType<int>());
}

void ConstantExpressionTester::TestDecimal() {
  ConstantExpression constantExpression(MakeNativeValue(3.14));
  CPPUNIT_ASSERT(constantExpression.GetType() == NativeDataType<double>());
  CPPUNIT_ASSERT(constantExpression.GetValue() == MakeNativeValue(3.14));
  Expression expression = constantExpression;
  CPPUNIT_ASSERT(expression->GetType() == NativeDataType<double>());
}

void ConstantExpressionTester::TestString() {
  ConstantExpression constantExpression(MakeNativeValue<string>(
    "hello world"));
  CPPUNIT_ASSERT(constantExpression.GetType() == NativeDataType<string>());
  CPPUNIT_ASSERT(constantExpression.GetValue() == MakeNativeValue<string>(
    "hello world"));
  Expression expression = constantExpression;
  CPPUNIT_ASSERT(expression->GetType() == NativeDataType<string>());
}
