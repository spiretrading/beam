#ifndef BEAM_QUERIES_GLOBAL_VARIABLE_DECLARATION_EXPRESSION_HPP
#define BEAM_QUERIES_GLOBAL_VARIABLE_DECLARATION_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Declares a global variable and evaluates an expression. */
  class GlobalVariableDeclarationExpression : public VirtualExpression {
    public:

      /**
       * Constructs a GlobalVariableDeclarationExpression.
       * @param name The name of the variable.
       * @param initial_value The variable's initial value.
       * @param body The body to evaluate.
       */
      GlobalVariableDeclarationExpression(
        std::string name, Expression initial_value, Expression body);

      /** Returns the name of the variable. */
      const std::string& get_name() const;

      /** Returns the variable's initial value. */
      const Expression& get_initial_value() const;

      /** Returns the body to evaluate. */
      const Expression& get_body() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      std::string m_name;
      Expression m_initial_value;
      Expression m_body;

      GlobalVariableDeclarationExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline GlobalVariableDeclarationExpression::
    GlobalVariableDeclarationExpression(
      std::string name, Expression initial_value, Expression body)
    : m_name(std::move(name)),
      m_initial_value(std::move(initial_value)),
      m_body(std::move(body)) {}

  inline const std::string&
      GlobalVariableDeclarationExpression::get_name() const {
    return m_name;
  }

  inline const Expression&
      GlobalVariableDeclarationExpression::get_initial_value() const {
    return m_initial_value;
  }

  inline const Expression&
      GlobalVariableDeclarationExpression::get_body() const {
    return m_body;
  }

  inline std::type_index GlobalVariableDeclarationExpression::get_type() const {
    return m_body.get_type();
  }

  inline void GlobalVariableDeclarationExpression::apply(
      ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& GlobalVariableDeclarationExpression::to_stream(
      std::ostream& out) const {
    return out << "(global (" << m_name << " " << m_initial_value << ") " <<
      m_body << ')';
  }

  inline GlobalVariableDeclarationExpression::
    GlobalVariableDeclarationExpression()
    : GlobalVariableDeclarationExpression(
        "", ConstantExpression(false), ConstantExpression(false)) {}

  template<IsShuttle S>
  void GlobalVariableDeclarationExpression::shuttle(
      S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("initial_value", m_initial_value);
    shuttle.shuttle("body", m_body);
  }

  inline void ExpressionVisitor::visit(
      const GlobalVariableDeclarationExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
