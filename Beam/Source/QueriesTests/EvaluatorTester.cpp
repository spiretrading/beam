#include <cstdint>
#include <limits>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/Evaluator.hpp"

using namespace Beam;
using namespace boost::posix_time;

TEST_SUITE("Evaluator") {
  TEST_CASE("constant_expression") {
    auto evaluator = translate(ConstantExpression(123));
    REQUIRE(evaluator->eval<int>() == 123);
  }

  TEST_CASE("and_expression") {
    auto evaluator_false =
      translate(ConstantExpression(true) && ConstantExpression(false));
    REQUIRE(!evaluator_false->eval<bool>());
    auto evaluator_true =
      translate(ConstantExpression(true) && ConstantExpression(true));
    REQUIRE(evaluator_true->eval<bool>());
  }

  TEST_CASE("or_expression") {
    auto evaluator_false =
      translate(ConstantExpression(false) || ConstantExpression(false));
    REQUIRE(!evaluator_false->eval<bool>());
    auto evaluator_true =
      translate(ConstantExpression(true) || ConstantExpression(false));
    REQUIRE(evaluator_true->eval<bool>());
  }

  TEST_CASE("not_expression") {
    auto evaluator_false = translate(!ConstantExpression(true));
    REQUIRE(!evaluator_false->eval<bool>());
    auto evaluator_true = translate(!ConstantExpression(false));
    REQUIRE(evaluator_true->eval<bool>());
  }

  TEST_CASE("signed_and_unsigned_comparison") {
    REQUIRE(translate(ConstantExpression(-1) <
      ConstantExpression(std::uint64_t(5)))->eval<bool>());
    REQUIRE(!translate(ConstantExpression(-1) == ConstantExpression(
      std::numeric_limits<std::uint64_t>::max()))->eval<bool>());
    REQUIRE(translate(ConstantExpression(13) ==
      ConstantExpression(std::uint64_t(13)))->eval<bool>());
  }

  TEST_CASE("addition_expression") {
    auto evaluator =
      translate(ConstantExpression(123) + ConstantExpression(321));
    REQUIRE(evaluator->eval<int>() == 444);
    auto mixed_evaluator =
      translate(ConstantExpression(123) + ConstantExpression(10.5));
    REQUIRE(mixed_evaluator->eval<double>() == 133.5);
  }

  TEST_CASE("subtraction_expression") {
    auto evaluator =
      translate(ConstantExpression(321) - ConstantExpression(123));
    REQUIRE(evaluator->eval<int>() == 198);
    auto mixed_evaluator =
      translate(ConstantExpression(123) - ConstantExpression(10.5));
    REQUIRE(mixed_evaluator->eval<double>() == 112.5);
  }

  TEST_CASE("multiplication_expression") {
    auto evaluator =
      translate(ConstantExpression(12) * ConstantExpression(11));
    REQUIRE(evaluator->eval<int>() == 132);
    auto mixed_evaluator =
      translate(ConstantExpression(12) * ConstantExpression(0.5));
    REQUIRE(mixed_evaluator->eval<double>() == 6.0);
  }

  TEST_CASE("division_expression") {
    auto evaluator =
      translate(ConstantExpression(132) / ConstantExpression(12));
    REQUIRE(evaluator->eval<int>() == 11);
    auto mixed_evaluator =
      translate(ConstantExpression(5) / ConstantExpression(2.0));
    REQUIRE(mixed_evaluator->eval<double>() == 2.5);
  }

  TEST_CASE("less_than_expression") {
    auto evaluator = translate(ConstantExpression(1) < ConstantExpression(2));
    REQUIRE(evaluator->eval<bool>());
    auto evaluator_reverse =
      translate(ConstantExpression(2) < ConstantExpression(1));
    REQUIRE(!evaluator_reverse->eval<bool>());
    auto evaluator_equal =
      translate(ConstantExpression(1) < ConstantExpression(1));
    REQUIRE(!evaluator_equal->eval<bool>());
  }

  TEST_CASE("mixed_operand_types") {
    SUBCASE("promoted") {
      auto evaluator =
        translate(ConstantExpression(1) < ConstantExpression(2.5));
      REQUIRE(evaluator->eval<bool>());
    }

    SUBCASE("reversed") {
      auto evaluator =
        translate(ConstantExpression(2.5) < ConstantExpression(1));
      REQUIRE(!evaluator->eval<bool>());
    }

    SUBCASE("equals") {
      auto evaluator =
        translate(ConstantExpression(1) == ConstantExpression(1.0));
      REQUIRE(evaluator->eval<bool>());
    }

    SUBCASE("equals_reversed") {
      auto evaluator =
        translate(ConstantExpression(1.0) == ConstantExpression(1));
      REQUIRE(evaluator->eval<bool>());
    }

    SUBCASE("not_equals_reversed") {
      auto evaluator =
        translate(ConstantExpression(2.5) != ConstantExpression(1));
      REQUIRE(evaluator->eval<bool>());
    }

    SUBCASE("signed_and_unsigned_arithmetic") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(1), ConstantExpression(std::uint64_t(2))};
      auto expression =
        FunctionExpression(ADDITION_NAME, typeid(std::uint64_t), parameters);
      REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
    }

    SUBCASE("unsupported") {
      auto parameters = std::vector<Expression>{
        ConstantExpression(1), ConstantExpression(std::string("2"))};
      auto expression = FunctionExpression(LESS_NAME, typeid(bool), parameters);
      REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
    }
  }

  TEST_CASE("time_arithmetic") {
    auto start = time_from_string("2020-01-02 00:00:00");
    auto finish = time_from_string("2020-01-02 00:00:02");
    auto second = duration_from_string("00:00:01");
    SUBCASE("difference") {
      auto evaluator =
        translate(ConstantExpression(finish) - ConstantExpression(start));
      REQUIRE(evaluator->eval<time_duration>() == seconds(2));
    }

    SUBCASE("difference_compared_to_duration") {
      auto difference = ConstantExpression(finish) - ConstantExpression(start);
      auto evaluator = translate(difference > ConstantExpression(second));
      REQUIRE(evaluator->eval<bool>());
    }

    SUBCASE("sum") {
      auto evaluator =
        translate(ConstantExpression(start) + ConstantExpression(second));
      REQUIRE(evaluator->eval<ptime>() ==
        time_from_string("2020-01-02 00:00:01"));
    }
  }

  TEST_CASE("less_than_or_equal_expression") {
    auto evaluator_equal =
      translate(ConstantExpression(1) <= ConstantExpression(1));
    REQUIRE(evaluator_equal->eval<bool>());
    auto evaluator_less =
      translate(ConstantExpression(1) <= ConstantExpression(2));
    REQUIRE(evaluator_less->eval<bool>());
    auto evaluator_greater =
      translate(ConstantExpression(2) <= ConstantExpression(1));
    REQUIRE(!evaluator_greater->eval<bool>());
  }

  TEST_CASE("equals_expression_constants") {
    auto evaluator_true =
      translate(ConstantExpression(3) == ConstantExpression(3));
    REQUIRE(evaluator_true->eval<bool>());
    auto evaluator_false =
      translate(ConstantExpression(3) == ConstantExpression(4));
    REQUIRE(!evaluator_false->eval<bool>());
  }

  TEST_CASE("not_equals_expression_constants") {
    auto evaluator_true =
      translate(ConstantExpression(3) != ConstantExpression(4));
    REQUIRE(evaluator_true->eval<bool>());
    auto evaluator_false =
      translate(ConstantExpression(3) != ConstantExpression(3));
    REQUIRE(!evaluator_false->eval<bool>());
  }

  TEST_CASE("greater_than_or_equal_expression") {
    auto evaluator_greater =
      translate(ConstantExpression(2) >= ConstantExpression(1));
    REQUIRE(evaluator_greater->eval<bool>());
    auto evaluator_equal =
      translate(ConstantExpression(1) >= ConstantExpression(1));
    REQUIRE(evaluator_equal->eval<bool>());
    auto evaluator_less =
      translate(ConstantExpression(1) >= ConstantExpression(2));
    REQUIRE(!evaluator_less->eval<bool>());
  }

  TEST_CASE("greater_than_expression") {
    auto evaluator = translate(ConstantExpression(2) > ConstantExpression(1));
    REQUIRE(evaluator->eval<bool>());
    auto evaluator_reverse =
      translate(ConstantExpression(1) > ConstantExpression(2));
    REQUIRE(!evaluator_reverse->eval<bool>());
    auto evaluator_equal =
      translate(ConstantExpression(1) > ConstantExpression(1));
    REQUIRE(!evaluator_equal->eval<bool>());
  }

  TEST_CASE("max_expression") {
    auto max_evaluator =
      translate(max(ConstantExpression(1), ConstantExpression(2)));
    REQUIRE(max_evaluator->eval<int>() == 2);
    auto max_equal =
      translate(max(ConstantExpression(3), ConstantExpression(3)));
    REQUIRE(max_equal->eval<int>() == 3);
    auto max_negative =
      translate(max(ConstantExpression(-5), ConstantExpression(-2)));
    REQUIRE(max_negative->eval<int>() == -2);
    auto max_double =
      translate(max(ConstantExpression(1.5), ConstantExpression(2.25)));
    REQUIRE(max_double->eval<double>() == 2.25);
    auto max_mixed =
      translate(max(ConstantExpression(1), ConstantExpression(2.5)));
    REQUIRE(max_mixed->eval<double>() == 2.5);
    auto max_mixed_reversed =
      translate(max(ConstantExpression(2.5), ConstantExpression(1)));
    REQUIRE(max_mixed_reversed->eval<double>() == 2.5);
  }

  TEST_CASE("min_expression") {
    auto min_evaluator =
      translate(min(ConstantExpression(1), ConstantExpression(2)));
    REQUIRE(min_evaluator->eval<int>() == 1);
    auto min_equal =
      translate(min(ConstantExpression(3), ConstantExpression(3)));
    REQUIRE(min_equal->eval<int>() == 3);
    auto min_negative =
      translate(min(ConstantExpression(-5), ConstantExpression(-2)));
    REQUIRE(min_negative->eval<int>() == -5);
    auto min_double =
      translate(min(ConstantExpression(1.5), ConstantExpression(2.25)));
    REQUIRE(min_double->eval<double>() == 1.5);
    auto min_mixed =
      translate(min(ConstantExpression(1), ConstantExpression(2.5)));
    REQUIRE(min_mixed->eval<double>() == 1.0);
    auto min_mixed_reversed =
      translate(min(ConstantExpression(2.5), ConstantExpression(1)));
    REQUIRE(min_mixed_reversed->eval<double>() == 1.0);
  }

  TEST_CASE("parameter_expression") {
    auto equals = ParameterExpression(0, typeid(bool)) ==
      ParameterExpression(1, typeid(bool));
    auto evaluator = translate(equals, typeid(bool));
    REQUIRE(evaluator->eval<bool>(false, false));
    REQUIRE(!evaluator->eval<bool>(false, true));
    REQUIRE(!evaluator->eval<bool>(true, false));
    REQUIRE(evaluator->eval<bool>(true, true));
  }

  TEST_CASE("reduce_expression") {
    auto sum =
      ParameterExpression(0, typeid(int)) + ParameterExpression(1, typeid(int));
    auto initial_value = Value(0);
    auto reducer =
      ReduceExpression(sum, ParameterExpression(0, typeid(int)), initial_value);
    auto evaluator = translate(reducer, typeid(int));
    REQUIRE(evaluator->eval<int>(1) == 1);
    REQUIRE(evaluator->eval<int>(1) == 2);
    REQUIRE(evaluator->eval<int>(2) == 4);
    REQUIRE(evaluator->eval<int>(5) == 9);
  }

  TEST_CASE("set_variable_without_declaration_throws") {
    auto expression = SetVariableExpression("x", ConstantExpression(1));
    REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
  }

  TEST_CASE("variable_without_declaration_throws") {
    auto expression = VariableExpression("x", typeid(int));
    REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
  }

  TEST_CASE("set_variable_in_global_declaration_body") {
    auto expression = GlobalVariableDeclarationExpression("x",
      ConstantExpression(1), SetVariableExpression("x", ConstantExpression(5)));
    auto evaluator = translate(expression);
    REQUIRE(evaluator->eval<int>() == 5);
  }

  TEST_CASE("variable_read_in_global_declaration_body") {
    auto expression = GlobalVariableDeclarationExpression(
      "v", ConstantExpression(9), VariableExpression("v", typeid(int)));
    auto evaluator = translate(expression);
    REQUIRE(evaluator->eval<int>() == 9);
  }

  TEST_CASE("out_of_range_parameter_expression") {
    {
      auto parameter =
        ParameterExpression(MAX_EVALUATOR_PARAMETERS, typeid(bool));
      REQUIRE_THROWS_AS(translate(parameter, typeid(bool)),
        ExpressionTranslationException);
    }
    {
      auto parameter = ParameterExpression(-1, typeid(bool));
      REQUIRE_THROWS_AS(translate(parameter, typeid(bool)),
        ExpressionTranslationException);
    }
  }

  TEST_CASE("type_check") {
    auto expression = ParameterExpression(0, typeid(int)) ==
      ParameterExpression(1, typeid(std::string));
    REQUIRE_THROWS_AS(translate(expression, typeid(int)),
      ExpressionTranslationException);
  }

  TEST_CASE("mismatched_type_parameter_expressions") {
    auto left = ParameterExpression(0, typeid(int)) ==
      ParameterExpression(1, typeid(int));
    auto right = ParameterExpression(0, typeid(double)) ==
      ParameterExpression(1, typeid(double));
    REQUIRE_THROWS_AS(translate(left || right, typeid(int)),
      ExpressionTranslationException);
  }

  TEST_CASE("parameter_expression_gap") {
    auto parameter = ParameterExpression(1, typeid(bool));
    REQUIRE_THROWS_AS(translate(parameter, typeid(bool)),
      ExpressionTranslationException);
  }

  TEST_CASE("mismatched_parameter_type") {
    SUBCASE("first_parameter") {
      auto evaluator = translate(
        ParameterExpression(0, typeid(int)) == ConstantExpression(0),
        typeid(int));
      REQUIRE_THROWS_AS(
        evaluator->eval<bool>(std::string()), TypeCompatibilityException);
    }
    SUBCASE("second_parameter") {
      auto evaluator = translate(ParameterExpression(0, typeid(int)) ==
        ParameterExpression(1, typeid(int)), typeid(int));
      REQUIRE_THROWS_AS(
        evaluator->eval<bool>(0, std::string()), TypeCompatibilityException);
    }
    SUBCASE("matching_parameter") {
      auto evaluator = translate(
        ParameterExpression(0, typeid(int)) == ConstantExpression(0),
        typeid(int));
      REQUIRE(evaluator->eval<bool>(0));
    }
  }

  TEST_CASE("mismatched_expression_type") {
    SUBCASE("function") {
      auto expression = FunctionExpression(EQUALS_NAME, typeid(int),
        {ConstantExpression(0), ConstantExpression(0)});
      REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
    }
    SUBCASE("and_operand") {
      auto operand = FunctionExpression(ADDITION_NAME, typeid(bool),
        {ConstantExpression(1), ConstantExpression(2)});
      REQUIRE_THROWS_AS(translate(operand && ConstantExpression(true)),
        ExpressionTranslationException);
    }
    SUBCASE("or_operand") {
      auto operand = FunctionExpression(ADDITION_NAME, typeid(bool),
        {ConstantExpression(1), ConstantExpression(2)});
      REQUIRE_THROWS_AS(translate(operand || ConstantExpression(false)),
        ExpressionTranslationException);
    }
    SUBCASE("not_operand") {
      auto operand = FunctionExpression(ADDITION_NAME, typeid(bool),
        {ConstantExpression(1), ConstantExpression(2)});
      REQUIRE_THROWS_AS(translate(!operand), ExpressionTranslationException);
    }
    SUBCASE("reduce_series") {
      auto reducer = ParameterExpression(0, typeid(int)) +
        ParameterExpression(1, typeid(int));
      auto series = FunctionExpression(EQUALS_NAME, typeid(int),
        {ConstantExpression(0), ConstantExpression(0)});
      auto expression = ReduceExpression(reducer, series, Value(0));
      REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
    }
    SUBCASE("set_variable_value") {
      auto value = FunctionExpression(ADDITION_NAME, typeid(std::string),
        {ConstantExpression(1), ConstantExpression(2)});
      auto expression = GlobalVariableDeclarationExpression("v",
        ConstantExpression(std::string()), SetVariableExpression("v", value));
      REQUIRE_THROWS_AS(translate(expression), ExpressionTranslationException);
    }
  }
}
