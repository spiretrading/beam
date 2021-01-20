#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("NotExpression") {
  TEST_CASE("constructor") {
    {
      auto notExpression = NotExpression(ConstantExpression(true));
      REQUIRE(notExpression.GetType() == BoolType());
      auto operandValue = notExpression.GetOperand().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(operandValue == NativeValue(true));
    }
    {
      auto notExpression = NotExpression(ConstantExpression(false));
      REQUIRE(notExpression.GetType() == BoolType());
      auto operandValue = notExpression.GetOperand().StaticCast<
        ConstantExpression>().GetValue().StaticCast<BoolValue>();
      REQUIRE(operandValue == NativeValue(false));
    }
  }

  TEST_CASE("invalid_constructor") {
    REQUIRE_THROWS_AS(NotExpression(ConstantExpression(0)),
      TypeCompatibilityException);
  }
}
