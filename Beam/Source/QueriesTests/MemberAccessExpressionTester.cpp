#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("MemberAccessExpression") {
  TEST_CASE("constructor") {
    auto memberA = MemberAccessExpression("a", BoolType(),
      ConstantExpression(false));
    REQUIRE(memberA.GetName() == "a");
    REQUIRE(memberA.GetType() == BoolType());
    auto expressionA = memberA.GetExpression().StaticCast<ConstantExpression>();
    REQUIRE(expressionA.GetValue()->GetValue<bool>() == false);
    auto memberB = MemberAccessExpression("b", IntType(),
      ConstantExpression(123));
    REQUIRE(memberB.GetName() == "b");
    REQUIRE(memberB.GetType() == IntType());
    auto expressionB = memberB.GetExpression().StaticCast<ConstantExpression>();
    REQUIRE(expressionB.GetValue()->GetValue<int>() == 123);
  }
}
