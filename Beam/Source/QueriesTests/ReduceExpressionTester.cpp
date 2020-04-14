#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardValues.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("ReduceExpression") {
  TEST_CASE("compatible_type_constructor") {
    auto intReducer = ConstantExpression(1);
    auto intSeries = ConstantExpression(2);
    auto initialInt = NativeValue(3);
    auto reduceInt = ReduceExpression(intReducer, intSeries, initialInt);
    REQUIRE(reduceInt.GetType() == IntType());
    REQUIRE(reduceInt.GetReduceExpression().StaticCast<
      ConstantExpression>().GetValue() ==
      intReducer.GetValue().StaticCast<IntValue>());
    REQUIRE(reduceInt.GetSeriesExpression().StaticCast<
      ConstantExpression>().GetValue() ==
      intSeries.GetValue().StaticCast<IntValue>());
    REQUIRE(reduceInt.GetInitialValue() == initialInt);
    ConstantExpression stringReducer = ConstantExpression(
      std::string("hello world"));
    ConstantExpression stringSeries = ConstantExpression(
      std::string("goodbye sky"));
    auto initialString = NativeValue(std::string("foo bar"));
    auto reduceString = ReduceExpression(stringReducer, stringSeries,
      initialString);
    REQUIRE(reduceString.GetType() == StringType());
    REQUIRE(reduceString.GetReduceExpression().StaticCast<
      ConstantExpression>().GetValue() ==
      stringReducer.GetValue().StaticCast<StringValue>());
    REQUIRE(reduceString.GetSeriesExpression().StaticCast<
      ConstantExpression>().GetValue() ==
      stringSeries.GetValue().StaticCast<StringValue>());
    REQUIRE(reduceString.GetInitialValue() == initialString);
  }

  TEST_CASE("incompatible_type_constructor") {
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(0),
      ConstantExpression(0), StringValue()), TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(0),
      ConstantExpression(std::string()), IntValue()),
      TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(std::string("")),
      ConstantExpression(0), IntValue()), TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(0),
      ConstantExpression(std::string()), StringValue()),
      TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(std::string()),
      ConstantExpression(0), StringValue()), TypeCompatibilityException);
    REQUIRE_THROWS_AS(ReduceExpression(ConstantExpression(std::string()),
      ConstantExpression(std::string()), IntValue()),
      TypeCompatibilityException);
  }
}
