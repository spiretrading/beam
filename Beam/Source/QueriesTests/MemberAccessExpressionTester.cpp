#include "Beam/QueriesTests/MemberAccessExpressionTester.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void MemberAccessExpressionTester::TestConstructor() {
  MemberAccessExpression memberA("a", BoolType(),
    MakeConstantExpression(false));
  CPPUNIT_ASSERT(memberA.GetName() == "a");
  CPPUNIT_ASSERT(memberA.GetType() == BoolType());
  ConstantExpression expressionA =
    memberA.GetExpression().StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(expressionA.GetValue()->GetValue<bool>() == false);
  MemberAccessExpression memberB("b", IntType(),
    MakeConstantExpression(123));
  CPPUNIT_ASSERT(memberB.GetName() == "b");
  CPPUNIT_ASSERT(memberB.GetType() == IntType());
  ConstantExpression expressionB =
    memberB.GetExpression().StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(expressionB.GetValue()->GetValue<int>() == 123);
}
