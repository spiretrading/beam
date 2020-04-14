#include <doctest/doctest.h>
#include "Beam/QueriesTests/SqlTranslatorTester.hpp"
#include "Beam/Queries/SqlTranslator.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;

TEST_SUITE("SqlTranslator") {
  TEST_CASE("or_expression") {
    vector<Expression> subExpressions;
    subExpressions.push_back(ConstantExpression{true});
    subExpressions.push_back(ConstantExpression{false});
    subExpressions.push_back(ConstantExpression{true});
    auto expression = MakeOrExpression(
      subExpressions.begin(), subExpressions.end());
    SqlTranslator translator{"test_table", expression};
    auto query = translator.Build();
  }
}
