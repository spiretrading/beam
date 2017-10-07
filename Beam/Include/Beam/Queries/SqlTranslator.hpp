#ifndef BEAM_SQLEXPRESSIONTRANSLATOR_HPP
#define BEAM_SQLEXPRESSIONTRANSLATOR_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/MySql/PosixTimeToMySqlDateTime.hpp"
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

namespace Beam {
namespace Queries {

  /*! \class SqlTranslator
      \brief Translates an Expression into an SQL query.
   */
  class SqlTranslator : public ExpressionVisitor {
    public:

      //! Constructs an SqlTranslator.
      /*!
        \param parameter The parameter/table name.
        \param expression The Expression to translate.
      */
      SqlTranslator(std::string parameter, Expression expression);

      //! Builds the SQL query.
      std::string BuildQuery();

      virtual void Visit(const ConstantExpression& expression);

      virtual void Visit(const FunctionExpression& expression);

      virtual void Visit(const OrExpression& expression);

      virtual void Visit(const ParameterExpression& expression);

      virtual void Visit(const VirtualExpression& expression);

    protected:

      //! Returns the current query being translated.
      std::string& GetQuery();

      //! Returns the parameter.
      const std::string& GetParameter() const;

    private:
      std::string m_parameter;
      Expression m_expression;
      std::string m_query;

      static std::string Escape(const std::string& source);
  };

  //! Translates an Expression into an SQL query.
  /*!
    \param parameter The parameter/table name.
    \param expression The Expression to translate.
    \return The SQL query representing the <i>expression</i>.
  */
  template<typename Translator = SqlTranslator>
  inline std::string BuildSqlQuery(std::string parameter,
      Expression expression) {
    Translator translator{std::move(parameter), std::move(expression)};
    return translator.BuildQuery();
  }

  inline SqlTranslator::SqlTranslator(std::string parameter,
      Expression expression)
      : m_parameter{std::move(parameter)},
        m_expression{std::move(expression)} {}

  inline std::string SqlTranslator::BuildQuery() {
    m_query.clear();
    m_expression->Apply(*this);
    return std::move(m_query);
  }

  inline void SqlTranslator::Visit(const ConstantExpression& expression) {
    auto& value = expression.GetValue();
    if(value->GetType()->GetNativeType() == typeid(bool)) {
      if(value->GetValue<bool>()) {
        GetQuery() += "true";
      } else {
        GetQuery() += "false";
      }
    } else if(value->GetType()->GetNativeType() == typeid(char)) {
      std::string escapeValue;
      escapeValue += value->GetValue<char>();
      GetQuery() += "\'";
      GetQuery() += Escape(escapeValue);
      GetQuery() += + "\'";
    } else if(value->GetType()->GetNativeType() == typeid(int)) {
      GetQuery() += boost::lexical_cast<std::string>(value->GetValue<int>());
    } else if(value->GetType()->GetNativeType() == typeid(double)) {
      GetQuery() += boost::lexical_cast<std::string>(value->GetValue<double>());
    } else if(value->GetType()->GetNativeType() == typeid(std::string)) {
      GetQuery() += "\'" + Escape(value->GetValue<std::string>()) + "\'";
    } else if(value->GetType()->GetNativeType() ==
        typeid(boost::posix_time::ptime)) {
      auto timestamp = MySql::ToMySqlTimestamp(
        value->GetValue<boost::posix_time::ptime>());
      GetQuery() += boost::lexical_cast<std::string>(timestamp);
    }
  }

  inline void SqlTranslator::Visit(const FunctionExpression& expression) {
    if(expression.GetName() == ADDITION_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException{
          "Invalid parameters."});
      }
      const auto& leftExpression = *expression.GetParameters()[0];
      leftExpression.Apply(*this);
      auto leftQuery = GetQuery();
      GetQuery().clear();
      const auto& rightExpression = *expression.GetParameters()[1];
      rightExpression.Apply(*this);
      auto rightQuery = GetQuery();
      GetQuery().clear();
      GetQuery() = "(" + leftQuery + " + " + rightQuery + ")";
    } else if(expression.GetName() == EQUALS_NAME) {
      if(expression.GetParameters().size() != 2) {
        BOOST_THROW_EXCEPTION(ExpressionTranslationException(
          "Invalid parameters."));
      }
      const auto& leftExpression = *expression.GetParameters()[0];
      leftExpression.Apply(*this);
      auto leftQuery = GetQuery();
      GetQuery().clear();
      const auto& rightExpression = *expression.GetParameters()[1];
      rightExpression.Apply(*this);
      auto rightQuery = GetQuery();
      GetQuery().clear();
      GetQuery() = "(" + leftQuery + " = " + rightQuery + ")";
    } else {
      BOOST_THROW_EXCEPTION(ExpressionTranslationException{
        "Function not supported."});
    }
  }

  inline void SqlTranslator::Visit(const OrExpression& expression) {
    const auto& leftExpression = *expression.GetLeftExpression();
    leftExpression.Apply(*this);
    auto leftQuery = GetQuery();
    GetQuery().clear();
    const auto& rightExpression = *expression.GetRightExpression();
    rightExpression.Apply(*this);
    auto rightQuery = GetQuery();
    GetQuery().clear();
    GetQuery() = "(" + leftQuery + " OR " + rightQuery + ")";
  }

  inline void SqlTranslator::Visit(const ParameterExpression& expression) {
    GetQuery() = m_parameter;
  }

  inline void SqlTranslator::Visit(const VirtualExpression& expression) {
    BOOST_THROW_EXCEPTION(ExpressionTranslationException{
      "Invalid expression."});
  }

  inline std::string& SqlTranslator::GetQuery() {
    return m_query;
  }

  inline const std::string& SqlTranslator::GetParameter() const {
    return m_parameter;
  }

  inline std::string SqlTranslator::Escape(const std::string& source) {
    std::string result;
    for(auto c : source) {
      if(c == '\0') {
        result += "\\0";
      } else if(c == '\'') {
        result += "\\'";
      } else if(c == '\"') {
        result += "\\\"";
      } else if(c == '\x08') {
        result += "\\b";
      } else if(c == '\n') {
        result += "\\n";
      } else if(c == '\r') {
        result += "\\r";
      } else if(c == '\t') {
        result += "\\t";
      } else if(c == '\x1A') {
        result += "\\n";
      } else if(c == '\\') {
        result += "\\\\";
      } else {
        result += c;
      }
    }
    return result;
  }
}
}

#endif
