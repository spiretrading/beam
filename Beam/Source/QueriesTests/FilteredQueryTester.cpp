#include <doctest/doctest.h>
#include "Beam/Queries/FilteredQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("FilteredQuery") {
  TEST_CASE("default_constructor") {
    auto query = FilteredQuery();
    REQUIRE(typeid(*query.GetFilter()) == typeid(ConstantExpression));
    auto filter = query.GetFilter().StaticCast<ConstantExpression>();
    REQUIRE(filter.GetValue()->GetValue<bool>() == true);
  }

  TEST_CASE("filter_constructor") {
    auto query = FilteredQuery(ConstantExpression(false));
    REQUIRE(typeid(*query.GetFilter()) == typeid(ConstantExpression));
    auto filter = query.GetFilter().StaticCast<ConstantExpression>();
    REQUIRE(filter.GetValue()->GetValue<bool>() == false);
    try {
      auto invalidQuery = FilteredQuery(ConstantExpression(123));
      REQUIRE(false);
    } catch(const std::exception&) {}
  }

  TEST_CASE("set_filter") {
    auto query = FilteredQuery();
    auto filter = query.GetFilter().StaticCast<ConstantExpression>();
    REQUIRE(filter.GetValue()->GetValue<bool>() != false);
    query.SetFilter(ConstantExpression(false));
    filter = query.GetFilter().StaticCast<ConstantExpression>();
    REQUIRE(filter.GetValue()->GetValue<bool>() == false);
    REQUIRE_THROWS_AS(query.SetFilter(ConstantExpression(123)),
      std::runtime_error);
  }
}
