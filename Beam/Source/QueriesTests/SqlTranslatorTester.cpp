#include "Beam/QueriesTests/SqlTranslatorTester.hpp"
#include "Beam/Queries/SqlTranslator.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void SqlTranslatorTester::TestOrExpression() {
  vector<Expression> subExpressions;
  subExpressions.push_back(ConstantExpression{MakeNativeValue(true)});
  subExpressions.push_back(ConstantExpression{MakeNativeValue(false)});
  subExpressions.push_back(ConstantExpression{MakeNativeValue(true)});
  auto expression = MakeOrExpression(
    subExpressions.begin(), subExpressions.end());
  SqlTranslator translator{"test_table", expression};
  auto query = translator.BuildQuery();
  CPPUNIT_ASSERT(query == "(true OR (false OR true))");
}
