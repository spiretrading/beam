#include "Beam/QueriesTests/ReduceExpressionTester.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ReduceExpressionTester::TestCompatibleTypeConstructor() {
  ConstantExpression intReducer = MakeConstantExpression(1);
  ConstantExpression intSeries = MakeConstantExpression(2);
  IntValue initialInt = MakeNativeValue(3);
  ReduceExpression reduceInt(intReducer, intSeries, initialInt);
  CPPUNIT_ASSERT(reduceInt.GetType() == IntType());
  CPPUNIT_ASSERT(reduceInt.GetReduceExpression().StaticCast<
    ConstantExpression>().GetValue() ==
    intReducer.GetValue().StaticCast<IntValue>());
  CPPUNIT_ASSERT(reduceInt.GetSeriesExpression().StaticCast<
    ConstantExpression>().GetValue() ==
    intSeries.GetValue().StaticCast<IntValue>());
  CPPUNIT_ASSERT(reduceInt.GetInitialValue() == initialInt);
  ConstantExpression stringReducer = MakeConstantExpression<string>(
    "hello world");
  ConstantExpression stringSeries = MakeConstantExpression<string>(
    "goodbye sky");
  StringValue initialString = MakeNativeValue<string>("foo bar");
  ReduceExpression reduceString(stringReducer, stringSeries, initialString);
  CPPUNIT_ASSERT(reduceString.GetType() == StringType());
  CPPUNIT_ASSERT(reduceString.GetReduceExpression().StaticCast<
    ConstantExpression>().GetValue() ==
    stringReducer.GetValue().StaticCast<StringValue>());
  CPPUNIT_ASSERT(reduceString.GetSeriesExpression().StaticCast<
    ConstantExpression>().GetValue() ==
    stringSeries.GetValue().StaticCast<StringValue>());
  CPPUNIT_ASSERT(reduceString.GetInitialValue() == initialString);
}

void ReduceExpressionTester::TestIncompatibleTypeConstructor() {
  CPPUNIT_ASSERT_THROW(ReduceExpression(MakeConstantExpression(0),
    MakeConstantExpression(0), StringValue()), TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(ReduceExpression(MakeConstantExpression(0),
    MakeConstantExpression<string>(""), IntValue()),
    TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(ReduceExpression(MakeConstantExpression<string>(""),
    MakeConstantExpression(0), IntValue()), TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(ReduceExpression(MakeConstantExpression(0),
    MakeConstantExpression<string>(""), StringValue()),
    TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(ReduceExpression(MakeConstantExpression<string>(""),
    MakeConstantExpression(0), StringValue()), TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(ReduceExpression(MakeConstantExpression<string>(""),
    MakeConstantExpression<string>(""), IntValue()),
    TypeCompatibilityException);
}
