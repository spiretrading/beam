#include "Beam/QueriesTests/ParameterExpressionTester.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void ParameterExpressionTester::TestConstructor() {
  ParameterExpression parameterA(0, BoolType());
  CPPUNIT_ASSERT(parameterA.GetIndex() == 0);
  CPPUNIT_ASSERT(parameterA.GetType() == BoolType());
  ParameterExpression parameterB(1, DecimalType());
  CPPUNIT_ASSERT(parameterB.GetIndex() == 1);
  CPPUNIT_ASSERT(parameterB.GetType() == DecimalType());
  ParameterExpression parameterC(2, StringType());
  CPPUNIT_ASSERT(parameterC.GetIndex() == 2);
  CPPUNIT_ASSERT(parameterC.GetType() == StringType());
}
