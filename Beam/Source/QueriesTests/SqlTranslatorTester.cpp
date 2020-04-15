#include <doctest/doctest.h>
#include "Beam/Queries/SqlTranslator.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("SqlTranslator") {
  TEST_CASE("or_expression") {
    auto subExpressions = std::vector<Expression>();
    subExpressions.push_back(ConstantExpression(true));
    subExpressions.push_back(ConstantExpression(false));
    subExpressions.push_back(ConstantExpression(true));
    auto expression = MakeOrExpression(
      subExpressions.begin(), subExpressions.end());
    auto translator = SqlTranslator("test_table", expression);
    auto query = translator.Build();
  }
}
