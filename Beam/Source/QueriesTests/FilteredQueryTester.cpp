#include "Beam/QueriesTests/FilteredQueryTester.hpp"
#include "Beam/Queries/FilteredQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace std;

void FilteredQueryTester::TestDefaultConstructor() {
  FilteredQuery query;
  CPPUNIT_ASSERT(typeid(*query.GetFilter()) == typeid(ConstantExpression));
  ConstantExpression filter =
    query.GetFilter().StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(filter.GetValue()->GetValue<bool>() == true);
}

void FilteredQueryTester::TestFilterConstructor() {
  FilteredQuery query(MakeConstantExpression(false));
  CPPUNIT_ASSERT(typeid(*query.GetFilter()) == typeid(ConstantExpression));
  ConstantExpression filter =
    query.GetFilter().StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(filter.GetValue()->GetValue<bool>() == false);
  try {
    FilteredQuery invalidQuery(MakeConstantExpression(123));
    CPPUNIT_ASSERT(false);
  } catch(const std::exception&) {}
}

void FilteredQueryTester::TestSetFilter() {
  FilteredQuery query;
  ConstantExpression filter =
    query.GetFilter().StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(filter.GetValue()->GetValue<bool>() != false);
  query.SetFilter(MakeConstantExpression(false));
  filter = query.GetFilter().StaticCast<ConstantExpression>();
  CPPUNIT_ASSERT(filter.GetValue()->GetValue<bool>() == false);
  CPPUNIT_ASSERT_THROW(query.SetFilter(MakeConstantExpression(123)),
    std::runtime_error);
}
