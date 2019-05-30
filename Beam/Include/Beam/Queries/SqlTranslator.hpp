#ifndef BEAM_SQL_TRANSLATOR_HPP
#define BEAM_SQL_TRANSLATOR_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include <Viper/Expressions/Expressions.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"

namespace Beam::Queries {

  /** Translates a query expression into an SQL expression. */
  class SqlTranslator : public ExpressionVisitor {
    public:

      //! Constructs an SqlTranslator.
      /*!
        \param parameter The parameter/table name.
        \param expression The Expression to translate.
      */
      SqlTranslator(std::string parameter, Expression expression);

      //! Builds the SQL expression.
      Viper::Expression Build();

      void Visit(const ConstantExpression& expression) override;

      void Visit(const FunctionExpression& expression) override;

      void Visit(const OrExpression& expression) override;

      void Visit(const ParameterExpression& expression) override;

      void Visit(const VirtualExpression& expression) override;

    protected:

      //! Returns the parameter.
      const Viper::Expression& GetParameter() const;

      //! Returns the current translation.
      Viper::Expression& GetTranslation();

    private:
      Viper::Expression m_parameter;
      Expression m_expression;
      Viper::Expression m_translation;
  };

  //! Translates a query expression into an SQL expression.
  /*!
    \param parameter The parameter/table name.
    \param expression The query expression to translate.
    \return The SQL expression.
  */
  template<typename Translator = SqlTranslator>
  auto BuildSqlQuery(std::string parameter, Expression expression) {
    Translator translator(std::move(parameter), std::move(expression));
    return translator.Build();
  }

  inline SqlTranslator::SqlTranslator(std::string parameter,
      Expression expression)
      : m_parameter(Viper::sym(std::move(parameter))),
        m_expression(std::move(expression)) {}

  inline Viper::Expression SqlTranslator::Build() {
    m_expression->Apply(*this);
    return std::move(GetTranslation());
  }

  inline void SqlTranslator::Visit(const ConstantExpression& expression) {
    auto& value = expression.GetValue();
    if(value->GetType()->GetNativeType() == typeid(bool)) {
      GetTranslation() = Viper::literal(value->GetValue<bool>());
    } else if(value->GetType()->GetNativeType() == typeid(char)) {
      GetTranslation() = Viper::literal(value->GetValue<char>());
    } else if(value->GetType()->GetNativeType() == typeid(int)) {
      GetTranslation() = Viper::literal(value->GetValue<int>());
    } else if(value->GetType()->GetNativeType() == typeid(double)) {
      GetTranslation() = Viper::literal(value->GetValue<double>());
    } else if(value->GetType()->GetNativeType() == typeid(std::string)) {
      GetTranslation() = Viper::literal(value->GetValue<std::string>());
    } else if(value->GetType()->GetNativeType() ==
        typeid(boost::posix_time::ptime)) {
      GetTranslation() = Viper::literal(
        value->GetValue<boost::posix_time::ptime>());
    }
  }

  inline void SqlTranslator::Visit(const FunctionExpression& expression) {
    if(expression.GetName() == ADDITION_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      expression.GetParameters()[0]->Apply(*this);
      auto leftTranslation = GetTranslation();
      expression.GetParameters()[1]->Apply(*this);
      auto rightTranslation = GetTranslation();
      GetTranslation() = leftTranslation + rightTranslation;
    } else if(expression.GetName() == EQUALS_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      expression.GetParameters()[0]->Apply(*this);
      auto leftTranslation = GetTranslation();
      expression.GetParameters()[1]->Apply(*this);
      auto rightTranslation = GetTranslation();
      GetTranslation() = leftTranslation == rightTranslation;
    } else {
      BOOST_THROW_EXCEPTION(ExpressionTranslationException(
        "Function not supported."));
    }
  }

  inline void SqlTranslator::Visit(const OrExpression& expression) {
    expression.GetLeftExpression()->Apply(*this);
    auto leftTranslation = GetTranslation();
    expression.GetRightExpression()->Apply(*this);
    auto rightTranslation = GetTranslation();
    GetTranslation() = leftTranslation || rightTranslation;
  }

  inline void SqlTranslator::Visit(const ParameterExpression& expression) {
    GetTranslation() = m_parameter;
  }

  inline void SqlTranslator::Visit(const VirtualExpression& expression) {
    BOOST_THROW_EXCEPTION(ExpressionTranslationException(
      "Invalid expression."));
  }

  inline const Viper::Expression& SqlTranslator::GetParameter() const {
    return m_parameter;
  }

  inline Viper::Expression& SqlTranslator::GetTranslation() {
    return m_translation;
  }
}

#endif
