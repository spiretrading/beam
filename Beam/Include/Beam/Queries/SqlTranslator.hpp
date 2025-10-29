#ifndef BEAM_SQL_TRANSLATOR_HPP
#define BEAM_SQL_TRANSLATOR_HPP
#include <string>
#include <boost/lexical_cast.hpp>
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
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"

namespace Beam {

  /** Translates a query expression into an SQL expression. */
  class SqlTranslator : protected ExpressionVisitor {
    public:

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

      /** Returns the current translation. */
      Viper::Expression& get_translation();

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
      Viper::Expression m_translation;

      template<typename F>
      void translate(const FunctionExpression& expression, F&& translation);
  };

  /**
   * Translates a query expression into an SQL expression.
   * @param parameter The parameter/table name.
   * @param expression The query expression to translate.
   * @return The SQL expression.
   */
  template<typename Translator = SqlTranslator>
  auto make_sql_query(std::string parameter, Expression expression) {
    auto translator = Translator(std::move(parameter), std::move(expression));
    return translator.make();
  }

  inline SqlTranslator::SqlTranslator(
    std::string parameter, Expression expression)
    : m_parameter(Viper::sym(std::move(parameter))),
      m_expression(std::move(expression)) {}

  inline Viper::Expression SqlTranslator::make() {
    m_expression.apply(*this);
    return get_translation();
  }

  inline void SqlTranslator::visit(const AndExpression& expression) {
    expression.get_left().apply(*this);
    auto left = get_translation();
    expression.get_right().apply(*this);
    auto right = get_translation();
    get_translation() = left && right;
  }

  inline void SqlTranslator::visit(const ConstantExpression& expression) {
    auto& value = expression.get_value();
    if(value.get_type() == typeid(bool)) {
      get_translation() = Viper::literal(value.as<bool>());
    } else if(value.get_type() == typeid(char)) {
      get_translation() = Viper::literal(value.as<char>());
    } else if(value.get_type() == typeid(int)) {
      get_translation() = Viper::literal(value.as<int>());
    } else if(value.get_type() == typeid(std::uint64_t)) {
      get_translation() = Viper::literal(value.as<std::uint64_t>());
    } else if(value.get_type() == typeid(double)) {
      get_translation() = Viper::literal(value.as<double>());
    } else if(value.get_type() == typeid(std::string)) {
      get_translation() = Viper::literal(value.as<std::string>());
    } else if(value.get_type() == typeid(boost::posix_time::ptime)) {
      get_translation() = Viper::literal(value.as<boost::posix_time::ptime>());
    }
  }

  inline void SqlTranslator::visit(const FunctionExpression& expression) {
    if(expression.get_name() == ADDITION_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left + right;
      });
    } else if(expression.get_name() == SUBTRACTION_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left - right;
      });
    } else if(expression.get_name() == MULTIPLICATION_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left * right;
      });
    } else if(expression.get_name() == DIVISION_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left / right;
      });
    } else if(expression.get_name() == LESS_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left < right;
      });
    } else if(expression.get_name() == LESS_EQUALS_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left <= right;
      });
    } else if(expression.get_name() == EQUALS_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left == right;
      });
    } else if(expression.get_name() == NOT_EQUALS_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left != right;
      });
    } else if(expression.get_name() == GREATER_EQUALS_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left >= right;
      });
    } else if(expression.get_name() == GREATER_NAME) {
      translate(expression, [] (auto&& left, auto&& right) {
        return left > right;
      });
    } else {
      boost::throw_with_location(
        ExpressionTranslationException("Function not supported."));
    }
  }

  inline void SqlTranslator::visit(const NotExpression& expression) {
    expression.get_operand().apply(*this);
    auto translation = get_translation();
    get_translation() = !translation;
  }

  inline void SqlTranslator::visit(const OrExpression& expression) {
    expression.get_left().apply(*this);
    auto left = get_translation();
    expression.get_right().apply(*this);
    auto right = get_translation();
    get_translation() = left || right;
  }

  inline void SqlTranslator::visit(const ParameterExpression& expression) {
    get_translation() = m_parameter;
  }

  inline void SqlTranslator::visit(const VirtualExpression& expression) {
    boost::throw_with_location(
      ExpressionTranslationException("Invalid expression."));
  }

  inline const Viper::Expression& SqlTranslator::get_parameter() const {
    return m_parameter;
  }

  inline Viper::Expression& SqlTranslator::get_translation() {
    return m_translation;
  }

  template<typename F>
  void SqlTranslator::translate(
      const FunctionExpression& expression, F&& translation) {
    if(expression.get_parameters().size() != 2) {
      boost::throw_with_location(
        ExpressionTranslationException("Invalid parameters."));
    }
    expression.get_parameters()[0].apply(*this);
    auto left = get_translation();
    expression.get_parameters()[1].apply(*this);
    auto right = get_translation();
    get_translation() =
      std::forward<F>(translation)(std::move(left), std::move(right));
  }
}

#endif
