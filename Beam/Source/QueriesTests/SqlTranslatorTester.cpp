#include <doctest/doctest.h>
#include "Beam/Queries/EvaluatorTranslator.hpp"
#include "Beam/Queries/SqlTranslator.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("SqlTranslator") {
  TEST_CASE("and_expression") {
    auto translation = make_sql_query(
      "p", ConstantExpression(true) && ConstantExpression(false));
    auto query = std::string();
    translation.append_query(query);
    REQUIRE(query == "(1 AND 0)");
  }

  TEST_CASE("constant_expression") {
    SUBCASE("bool") {
      auto translation = make_sql_query("p", ConstantExpression(true));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "1");
    }

    SUBCASE("char") {
      auto translation = make_sql_query("p", ConstantExpression('x'));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "\"x\"");
    }

    SUBCASE("int") {
      auto translation = make_sql_query("p", ConstantExpression(7));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "7");
    }

    SUBCASE("uint64_t") {
      auto value = std::uint64_t(1234567890123ull);
      auto translation = make_sql_query("p", ConstantExpression(value));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "1234567890123");
    }

    SUBCASE("double") {
      auto translation = make_sql_query("p", ConstantExpression(3.14));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "3.140000");
    }

    SUBCASE("posix_time") {
      auto time = time_from_string("2020-01-02 00:00:00");
      auto translation = make_sql_query("p", ConstantExpression(time));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == std::to_string(to_sql_timestamp(time)));
    }

    SUBCASE("string") {
      auto translation = make_sql_query("p", ConstantExpression("hello_world"));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "\"hello_world\"");
    }

    SUBCASE("time_duration") {
      auto duration = duration_from_string("01:30:00");
      auto translation = make_sql_query("p", ConstantExpression(duration));
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == std::to_string(duration.total_microseconds()));
    }
  }

  TEST_CASE("unsupported_constant_throws") {
    auto value =
      Value(std::make_shared<NativeValue<std::int16_t>>(std::int16_t(5)));
    REQUIRE_THROWS_AS(make_sql_query("p", ConstantExpression(value)),
      ExpressionTranslationException);
  }

  TEST_CASE("function_expressions") {
    SUBCASE("addition") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
      auto expression =
        FunctionExpression(ADDITION_NAME, typeid(int), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(1 + 2)");
    }

    SUBCASE("subtraction") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(5), ConstantExpression(3)};
      auto expression =
        FunctionExpression(SUBTRACTION_NAME, typeid(int), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(5 - 3)");
    }

    SUBCASE("multiplication") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(5), ConstantExpression(3)};
      auto expression =
        FunctionExpression(MULTIPLICATION_NAME, typeid(int), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(5 * 3)");
    }

    SUBCASE("division") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(6), ConstantExpression(2)};
      auto expression =
        FunctionExpression(DIVISION_NAME, typeid(int), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(6 / 2)");
    }

    SUBCASE("less") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
      auto expression = FunctionExpression(LESS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(1 < 2)");
    }

    SUBCASE("less_equals") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(2), ConstantExpression(2)};
      auto expression =
        FunctionExpression(LESS_EQUALS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(2 <= 2)");
    }

    SUBCASE("equals") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(1)};
      auto expression = FunctionExpression(EQUALS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(1 = 1)");
    }

    SUBCASE("not_equals") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
      auto expression =
        FunctionExpression(NOT_EQUALS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(1 <> 2)");
    }

    SUBCASE("greater_equals") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(5), ConstantExpression(4)};
      auto expression =
        FunctionExpression(GREATER_EQUALS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(5 >= 4)");
    }

    SUBCASE("greater") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(5), ConstantExpression(4)};
      auto expression =
        FunctionExpression(GREATER_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(5 > 4)");
    }
  }

  TEST_CASE("mixed_operand_types") {
    SUBCASE("promoted") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2.5)};
      auto expression =
        FunctionExpression(LESS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(1 < 2.500000)");
    }

    SUBCASE("reversed") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(2.5), ConstantExpression(1)};
      auto expression =
        FunctionExpression(LESS_NAME, typeid(bool), parameters);
      auto translation = make_sql_query("p", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "(2.500000 < 1)");
    }
  }

  TEST_CASE("not_expression") {
    auto translation =
      make_sql_query("p", NotExpression(ConstantExpression(true)));
    auto query = std::string();
    translation.append_query(query);
    REQUIRE(query == "(NOT 1)");
  }

  TEST_CASE("or_expression") {
    auto translation = make_sql_query(
      "p", ConstantExpression(true) || ConstantExpression(false));
    auto query = std::string();
    translation.append_query(query);
    REQUIRE(query == "(1 OR 0)");
  }

  TEST_CASE("parameter") {
    SUBCASE("valid") {
      auto expression = ParameterExpression(0, typeid(int));
      auto translation = make_sql_query("my_table", expression);
      auto query = std::string();
      translation.append_query(query);
      REQUIRE(query == "my_table");
    }

    SUBCASE("index_out_of_range") {
      auto expression =
        ParameterExpression(MAX_EVALUATOR_PARAMETERS, typeid(int));
      REQUIRE_THROWS_AS(
        make_sql_query("my_table", expression), ExpressionTranslationException);
    }

    SUBCASE("negative_index") {
      auto expression = ParameterExpression(-1, typeid(int));
      REQUIRE_THROWS_AS(
        make_sql_query("my_table", expression), ExpressionTranslationException);
    }

    SUBCASE("type_mismatch") {
      auto left = std::vector<Expression>{
        ParameterExpression(0, typeid(int)), ConstantExpression(1)};
      auto right = std::vector<Expression>{
        ParameterExpression(0, typeid(std::string)),
        ConstantExpression(std::string("hello"))};
      auto expression = AndExpression(
        FunctionExpression(EQUALS_NAME, typeid(bool), left),
        FunctionExpression(EQUALS_NAME, typeid(bool), right));
      REQUIRE_THROWS_AS(
        make_sql_query("my_table", expression), ExpressionTranslationException);
    }

    SUBCASE("missing") {
      auto parameters = std::vector<Expression>{
        ParameterExpression(1, typeid(int)), ConstantExpression(1)};
      auto expression =
        FunctionExpression(EQUALS_NAME, typeid(bool), parameters);
      REQUIRE_THROWS_AS(
        make_sql_query("my_table", expression), ExpressionTranslationException);
    }
  }

  TEST_CASE("unsupported_function_throws") {
    auto parameters =
      std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
    auto expression = FunctionExpression("unknown", typeid(int), parameters);
    REQUIRE_THROWS_AS(
      make_sql_query("p", expression), ExpressionTranslationException);
  }

  TEST_CASE("invalid_parameter_count_throws") {
    auto parameters = std::vector<Expression>{ConstantExpression(1)};
    auto expression =
      FunctionExpression(ADDITION_NAME, typeid(int), parameters);
    REQUIRE_THROWS_AS(
      make_sql_query("p", expression), ExpressionTranslationException);
  }

  TEST_CASE("mismatched_expression_type_throws") {
    SUBCASE("function") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
      auto expression =
        FunctionExpression(ADDITION_NAME, typeid(std::string), parameters);
      REQUIRE_THROWS_AS(
        make_sql_query("p", expression), ExpressionTranslationException);
    }

    SUBCASE("comparison") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
      auto expression =
        FunctionExpression(EQUALS_NAME, typeid(int), parameters);
      REQUIRE_THROWS_AS(
        make_sql_query("p", expression), ExpressionTranslationException);
    }

    SUBCASE("operand") {
      auto parameters =
        std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)};
      auto operand =
        FunctionExpression(ADDITION_NAME, typeid(bool), parameters);
      REQUIRE_THROWS_AS(make_sql_query("p", NotExpression(operand)),
        ExpressionTranslationException);
    }
  }

  TEST_CASE("incompatible_operand_types_throws") {
    SUBCASE("equals") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(1), ConstantExpression(std::string("hello"))};
      auto expression =
        FunctionExpression(EQUALS_NAME, typeid(bool), parameters);
      REQUIRE_THROWS_AS(
        make_sql_query("p", expression), ExpressionTranslationException);
    }

    SUBCASE("addition") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(1), ConstantExpression(std::string("hello"))};
      auto expression =
        FunctionExpression(ADDITION_NAME, typeid(int), parameters);
      REQUIRE_THROWS_AS(
        make_sql_query("p", expression), ExpressionTranslationException);
    }
  }

  TEST_CASE("visiting_unhandled_virtual_expression_throws") {
    struct TestVirtual : VirtualExpression {
      std::type_index get_type() const override {
        return typeid(bool);
      }

      void apply(ExpressionVisitor& visitor) const override {
        visitor.visit(*this);
      }
    };
    REQUIRE_THROWS_AS(
      make_sql_query("p", TestVirtual()), ExpressionTranslationException);
  }
}
