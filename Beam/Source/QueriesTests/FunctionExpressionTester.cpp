#include <doctest/doctest.h>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/FunctionExpression.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("FunctionExpression") {
  TEST_CASE("empty_function") {
    auto parameters = std::vector<Expression>();
    auto function = FunctionExpression("empty", DecimalType(), parameters);
    REQUIRE(function.GetName() == "empty");
    REQUIRE(function.GetType() == DecimalType());
    REQUIRE(function.GetParameters().empty());
  }

  TEST_CASE("unary_function") {
    auto parameters = std::vector<Expression>();
    parameters.push_back(ConstantExpression(std::string("hello world")));
    auto function = FunctionExpression("unary", BoolType(), parameters);
    REQUIRE(function.GetName() == "unary");
    REQUIRE(function.GetType() == BoolType());
    REQUIRE(function.GetParameters().size() == 1);
    REQUIRE(typeid(*function.GetParameters()[0]) == typeid(ConstantExpression));
    auto p1 = function.GetParameters()[0].StaticCast<ConstantExpression>();
    REQUIRE(p1.GetValue()->GetValue<std::string>() == "hello world");
  }

  TEST_CASE("binary_function") {
    auto parameters = std::vector<Expression>();
    parameters.push_back(ConstantExpression(5));
    parameters.push_back(ConstantExpression(6));
    auto function = FunctionExpression("binary", IntType(), parameters);
    REQUIRE(function.GetName() == "binary");
    REQUIRE(function.GetType() == IntType());
    REQUIRE(function.GetParameters().size() == 2);
    REQUIRE(typeid(*function.GetParameters()[0]) == typeid(ConstantExpression));
    auto p1 = function.GetParameters()[0].StaticCast<ConstantExpression>();
    REQUIRE(p1.GetValue()->GetValue<int>() == 5);
    REQUIRE(typeid(*function.GetParameters()[1]) == typeid(ConstantExpression));
    auto p2 = function.GetParameters()[1].StaticCast<ConstantExpression>();
    REQUIRE(p2.GetValue()->GetValue<int>() == 6);
  }
}
