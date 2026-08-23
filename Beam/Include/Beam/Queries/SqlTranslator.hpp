#ifndef BEAM_SQL_TRANSLATOR_HPP
#define BEAM_SQL_TRANSLATOR_HPP
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <boost/callable_traits/return_type.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/Expressions/Expressions.hpp>
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/QueryTypes.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Sql/Conversions.hpp"
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"
#include "Beam/Utilities/Instantiate.hpp"

namespace Beam {

  /** Stores an SQL expression and the type it evaluates to. */
  struct SqlTranslation {

    /** The SQL expression. */
    Viper::Expression m_expression;

    /** The type the expression evaluates to. */
    std::type_index m_type;
  };

  /**
   * Returns the type that a function's operands translate to.
   * @tparam F The function translator to evaluate.
   */
  template<typename F>
  struct FunctionTypeTranslator {
    using type = typename F::type;

    template<typename... Args>
    std::type_index operator ()() const {
      return typeid(std::remove_cvref_t<boost::callable_traits::return_type_t<
        typename F::template Operation<Args...>>>);
    }
  };

  /**
   * Translates a query expression into an SQL expression.
   * @tparam Q The list of types supported.
   */
  template<typename Q>
  class SqlTranslator : protected ExpressionVisitor {
    public:

      /** The list of types supported. */
      using QueryTypes = Q;

      /** Lists all native types. */
      using NativeTypes = typename QueryTypes::NativeTypes;

      /** Lists types that can be compared. */
      using ComparableTypes = typename QueryTypes::ComparableTypes;

      /**
       * Constructs an SqlTranslator.
       * @param parameter The parameter/table name.
       * @param expression The Expression to translate.
       */
      SqlTranslator(std::string parameter, Expression expression);

      /** Returns the SQL expression. */
      Viper::Expression make();

    protected:

      /** Returns the parameter. */
      const Viper::Expression& get_parameter() const;

      /**
       * Sets the most recently translated expression.
       * @param expression The most recently translated expression.
       * @param type The type the <i>expression</i> evaluates to.
       */
      void set_translation(Viper::Expression expression, std::type_index type);

      /**
       * Translates a sub-expression, ensuring its translation has the type the
       * sub-expression declares.
       * @param expression The sub-expression to translate.
       * @return The translation of the <i>expression</i>.
       */
      SqlTranslation translate(const Expression& expression);

      void visit(const AndExpression& expression) override;
      void visit(const ConstantExpression& expression) override;
      void visit(const FunctionExpression& expression) override;
      void visit(const NotExpression& expression) override;
      void visit(const OrExpression& expression) override;
      void visit(const ParameterExpression& expression) override;
      void visit(const VirtualExpression& expression) override;

    private:
      Viper::Expression m_parameter;
      Expression m_expression;
      SqlTranslation m_translation;
      std::array<boost::optional<std::type_index>, MAX_EVALUATOR_PARAMETERS>
        m_parameters;
      int m_max_parameter;

      static Viper::Expression convert_time_unit(
        Viper::Expression term, std::type_index from, std::type_index to);
      template<typename T, typename F>
      void translate(const FunctionExpression& expression, F&& translation);
  };

  /**
   * Translates a query expression into an SQL expression.
   * @param parameter The parameter/table name.
   * @param expression The query expression to translate.
   * @return The SQL expression.
   */
  template<typename Translator = SqlTranslator<QueryTypes>>
  auto make_sql_query(std::string parameter, Expression expression) {
    auto translator = Translator(std::move(parameter), std::move(expression));
    return translator.make();
  }

  template<typename Q>
  SqlTranslator<Q>::SqlTranslator(std::string parameter, Expression expression)
    : m_parameter(Viper::sym(std::move(parameter))),
      m_expression(std::move(expression)),
      m_translation(Viper::Expression(), typeid(void)),
      m_max_parameter(-1) {}

  template<typename Q>
  Viper::Expression SqlTranslator<Q>::make() {
    auto translation = translate(m_expression);
    for(auto i = 0; i <= m_max_parameter; ++i) {
      if(!m_parameters[i]) {
        boost::throw_with_location(
          ExpressionTranslationException("Missing parameter."));
      }
    }
    return std::move(translation.m_expression);
  }

  template<typename Q>
  SqlTranslation SqlTranslator<Q>::translate(const Expression& expression) {
    m_translation = SqlTranslation(Viper::Expression(), typeid(void));
    expression.apply(*this);
    if(m_translation.m_type != expression.get_type()) {
      boost::throw_with_location(
        ExpressionTranslationException("Expression type mismatch."));
    }
    return m_translation;
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const AndExpression& expression) {
    auto left = translate(expression.get_left());
    auto right = translate(expression.get_right());
    set_translation(left.m_expression && right.m_expression, typeid(bool));
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const ConstantExpression& expression) {
    auto& value = expression.get_value();
    if(value.get_type() == typeid(bool)) {
      set_translation(Viper::literal(value.as<bool>()), value.get_type());
    } else if(value.get_type() == typeid(char)) {
      set_translation(Viper::literal(value.as<char>()), value.get_type());
    } else if(value.get_type() == typeid(int)) {
      set_translation(Viper::literal(value.as<int>()), value.get_type());
    } else if(value.get_type() == typeid(std::uint64_t)) {
      set_translation(
        Viper::literal(value.as<std::uint64_t>()), value.get_type());
    } else if(value.get_type() == typeid(double)) {
      set_translation(Viper::literal(value.as<double>()), value.get_type());
    } else if(value.get_type() == typeid(std::string)) {
      set_translation(
        Viper::literal(value.as<std::string>()), value.get_type());
    } else if(value.get_type() == typeid(boost::posix_time::ptime)) {
      set_translation(
        Viper::literal(value.as<boost::posix_time::ptime>()), value.get_type());
    } else if(value.get_type() == typeid(boost::posix_time::time_duration)) {
      set_translation(
        Viper::literal(value.as<boost::posix_time::time_duration>()),
        value.get_type());
    } else {
      boost::throw_with_location(
        ExpressionTranslationException("Constant type not supported."));
    }
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const FunctionExpression& expression) {
    if(expression.get_name() == ADDITION_NAME) {
      translate<AdditionExpressionTranslator<NativeTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left + right;
        });
    } else if(expression.get_name() == SUBTRACTION_NAME) {
      translate<SubtractionExpressionTranslator<NativeTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left - right;
        });
    } else if(expression.get_name() == MULTIPLICATION_NAME) {
      translate<MultiplicationExpressionTranslator<NativeTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left * right;
        });
    } else if(expression.get_name() == DIVISION_NAME) {
      translate<DivisionExpressionTranslator<NativeTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left / right;
        });
    } else if(expression.get_name() == LESS_NAME) {
      translate<LessExpressionTranslator<ComparableTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left < right;
        });
    } else if(expression.get_name() == LESS_EQUALS_NAME) {
      translate<LessEqualsExpressionTranslator<ComparableTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left <= right;
        });
    } else if(expression.get_name() == EQUALS_NAME) {
      translate<EqualsExpressionTranslator<NativeTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left == right;
        });
    } else if(expression.get_name() == NOT_EQUALS_NAME) {
      translate<NotEqualsExpressionTranslator<NativeTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left != right;
        });
    } else if(expression.get_name() == GREATER_EQUALS_NAME) {
      translate<GreaterEqualsExpressionTranslator<ComparableTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left >= right;
        });
    } else if(expression.get_name() == GREATER_NAME) {
      translate<GreaterExpressionTranslator<ComparableTypes>>(expression,
        [] (auto&& left, auto&& right) {
          return left > right;
        });
    } else {
      boost::throw_with_location(
        ExpressionTranslationException("Function not supported."));
    }
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const NotExpression& expression) {
    auto operand = translate(expression.get_operand());
    set_translation(!operand.m_expression, typeid(bool));
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const OrExpression& expression) {
    auto left = translate(expression.get_left());
    auto right = translate(expression.get_right());
    set_translation(left.m_expression || right.m_expression, typeid(bool));
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const ParameterExpression& expression) {
    if(expression.get_index() < 0 ||
        expression.get_index() >= MAX_EVALUATOR_PARAMETERS) {
      boost::throw_with_location(
        ExpressionTranslationException("Too many parameters."));
    }
    auto& check = m_parameters[expression.get_index()];
    if(check && check != expression.get_type()) {
      boost::throw_with_location(
        ExpressionTranslationException("Parameter type mismatch."));
    }
    check = expression.get_type();
    m_max_parameter = std::max(m_max_parameter, expression.get_index());
    set_translation(m_parameter, expression.get_type());
  }

  template<typename Q>
  void SqlTranslator<Q>::visit(const VirtualExpression& expression) {
    boost::throw_with_location(
      ExpressionTranslationException("Invalid expression."));
  }

  template<typename Q>
  const Viper::Expression& SqlTranslator<Q>::get_parameter() const {
    return m_parameter;
  }

  template<typename Q>
  void SqlTranslator<Q>::set_translation(
      Viper::Expression expression, std::type_index type) {
    m_translation = SqlTranslation(std::move(expression), type);
  }

  template<typename Q>
  Viper::Expression SqlTranslator<Q>::convert_time_unit(
      Viper::Expression term, std::type_index from, std::type_index to) {
    const auto MICROSECONDS_PER_MILLISECOND = 1000;
    if(from == typeid(boost::posix_time::ptime) &&
        to == typeid(boost::posix_time::time_duration)) {
      return term * Viper::literal(MICROSECONDS_PER_MILLISECOND);
    } else if(from == typeid(boost::posix_time::time_duration) &&
        to == typeid(boost::posix_time::ptime)) {
      return term / Viper::literal(MICROSECONDS_PER_MILLISECOND);
    }
    return term;
  }

  template<typename Q>
  template<typename T, typename F>
  void SqlTranslator<Q>::translate(
      const FunctionExpression& expression, F&& translation) {
    if(expression.get_parameters().size() != 2) {
      boost::throw_with_location(
        ExpressionTranslationException("Invalid parameters."));
    }
    auto left = translate(expression.get_parameters()[0]);
    auto right = translate(expression.get_parameters()[1]);
    auto type = [&] {
      try {
        return instantiate<FunctionTypeTranslator<T>>(
          left.m_type, right.m_type)();
      } catch(const std::invalid_argument&) {
        std::throw_with_nested(
          ExpressionTranslationException("Type mismatch."));
      }
    }();
    set_translation(std::forward<F>(translation)(
      convert_time_unit(std::move(left.m_expression), left.m_type, type),
      convert_time_unit(std::move(right.m_expression), right.m_type, type)),
      type);
  }
}

#endif
