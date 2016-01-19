#include "Beam/QueriesTests/OrExpressionTester.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void OrExpressionTester::TestConstructor() {
  {
    OrExpression orExpression(MakeConstantExpression(true),
      MakeConstantExpression(true));
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(true));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(true));
  }
  {
    OrExpression orExpression(MakeConstantExpression(true),
      MakeConstantExpression(false));
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(true));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(false));
  }
  {
    OrExpression orExpression(MakeConstantExpression(false),
      MakeConstantExpression(true));
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(false));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(true));
  }
  {
    OrExpression orExpression(MakeConstantExpression(false),
      MakeConstantExpression(false));
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(false));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(false));
  }
}

void OrExpressionTester::TestInvalidConstructor() {
  CPPUNIT_ASSERT_THROW(OrExpression(MakeConstantExpression(0),
    MakeConstantExpression(true)), TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(OrExpression(MakeConstantExpression(true),
    MakeConstantExpression(0)), TypeCompatibilityException);
  CPPUNIT_ASSERT_THROW(OrExpression(MakeConstantExpression(0),
    MakeConstantExpression(0)), TypeCompatibilityException);
}

void OrExpressionTester::TestEmptyMakeOrExpression() {
  vector<Expression> subexpressions;
  auto orExpression = MakeOrExpression(subexpressions.begin(),
    subexpressions.end());
  CPPUNIT_ASSERT_NO_THROW(orExpression.DynamicCast<ConstantExpression>());
  CPPUNIT_ASSERT_NO_THROW(orExpression.StaticCast<
    ConstantExpression>().GetType().DynamicCast<BoolType>());
  CPPUNIT_ASSERT(orExpression.StaticCast<
    ConstantExpression>().GetValue()->GetValue<bool>() == false);
}

void OrExpressionTester::TestSingleMakeOrExpression() {
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(true));
    auto orExpression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    CPPUNIT_ASSERT_NO_THROW(orExpression.DynamicCast<ConstantExpression>());
    CPPUNIT_ASSERT_NO_THROW(orExpression.StaticCast<
      ConstantExpression>().GetType().DynamicCast<BoolType>());
    CPPUNIT_ASSERT(orExpression.StaticCast<
      ConstantExpression>().GetValue()->GetValue<bool>() == true);
  }
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(false));
    auto orExpression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    CPPUNIT_ASSERT_NO_THROW(orExpression.DynamicCast<ConstantExpression>());
    CPPUNIT_ASSERT_NO_THROW(orExpression.StaticCast<
      ConstantExpression>().GetType().DynamicCast<BoolType>());
    CPPUNIT_ASSERT(orExpression.StaticCast<
      ConstantExpression>().GetValue()->GetValue<bool>() == false);
  }
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(0));
    CPPUNIT_ASSERT_THROW(
      MakeOrExpression(subexpressions.begin(), subexpressions.end()),
      TypeCompatibilityException);
  }
}

void OrExpressionTester::TestTwoSubExpressionMakeOrExpression() {
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(false));
    subexpressions.push_back(MakeConstantExpression(false));
    auto expression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    CPPUNIT_ASSERT_NO_THROW(expression.DynamicCast<OrExpression>());
    auto orExpression = expression.StaticCast<OrExpression>();
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(false));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(false));
  }
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(false));
    subexpressions.push_back(MakeConstantExpression(true));
    auto expression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    CPPUNIT_ASSERT_NO_THROW(expression.DynamicCast<OrExpression>());
    auto orExpression = expression.StaticCast<OrExpression>();
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(false));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(true));
  }
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(true));
    subexpressions.push_back(MakeConstantExpression(false));
    auto expression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    CPPUNIT_ASSERT_NO_THROW(expression.DynamicCast<OrExpression>());
    auto orExpression = expression.StaticCast<OrExpression>();
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(true));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(false));
  }
  {
    vector<Expression> subexpressions;
    subexpressions.push_back(MakeConstantExpression(true));
    subexpressions.push_back(MakeConstantExpression(true));
    auto expression = MakeOrExpression(subexpressions.begin(),
      subexpressions.end());
    CPPUNIT_ASSERT_NO_THROW(expression.DynamicCast<OrExpression>());
    auto orExpression = expression.StaticCast<OrExpression>();
    CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
    auto leftValue = orExpression.GetLeftExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    auto rightValue = orExpression.GetRightExpression().StaticCast<
      ConstantExpression>().GetValue().StaticCast<BoolValue>();
    CPPUNIT_ASSERT(leftValue == MakeNativeValue(true));
    CPPUNIT_ASSERT(rightValue == MakeNativeValue(true));
  }
}

void OrExpressionTester::TestMultipleSubExpressionMakeOrExpression() {
  unsigned int LOWER_BOUND = 3;
  unsigned int UPPER_BOUND = 5;
  for(unsigned int i = LOWER_BOUND; i < UPPER_BOUND; ++i) {
    for(unsigned int j = 0; j < (1UL << i); ++j) {
      vector<Expression> subexpressions;
      for(unsigned int k = 0; k < i; ++k) {
        bool value = ((j & (1UL << k)) != 0);
        subexpressions.push_back(MakeConstantExpression(value));
      }
      auto expression = MakeOrExpression(subexpressions.begin(),
        subexpressions.end());
      CPPUNIT_ASSERT_NO_THROW(expression.DynamicCast<OrExpression>());
      auto orExpression = expression.StaticCast<OrExpression>();
      CPPUNIT_ASSERT(orExpression.GetType() == BoolType());
      const VirtualExpression* traversal = &orExpression;
      for(unsigned int k = 0; k < i - 1; ++k) {
        bool value = ((j & (1UL << k)) != 0);
        auto expression = static_cast<const OrExpression*>(traversal);
        CPPUNIT_ASSERT_NO_THROW(
          expression->GetLeftExpression().DynamicCast<ConstantExpression>());
        auto leftValue = expression->GetLeftExpression().StaticCast<
          ConstantExpression>().GetValue().StaticCast<BoolValue>();
        CPPUNIT_ASSERT(leftValue.GetValue<bool>() == value);
        traversal = &expression->GetRightExpression().StaticCast<
          VirtualExpression>();
      }
      bool value = ((j & (1UL << (i - 1))) != 0);
      auto finalExpression = static_cast<const ConstantExpression*>(traversal);
      auto finalValue = finalExpression->GetValue().StaticCast<BoolValue>();
      CPPUNIT_ASSERT(finalValue.GetValue<bool>() == value);
    }
  }
}
