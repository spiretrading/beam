#ifndef BEAM_QUERIES_SET_VARIABLE_EXPRESSION_HPP
#define BEAM_QUERIES_SET_VARIABLE_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Sets the value of a variable. */
  class SetVariableExpression : public VirtualExpression {
    public:

      /**
       * Constructs a SetVariableExpression.
       * @param name The name of the variable to set.
       * @param value The value to assign to the variable.
       */
      SetVariableExpression(std::string name, Expression value);

      /** Returns the name of the variable to set. */
      const std::string& get_name() const;

      /** Returns the value to set the variable to. */
      const Expression& get_value() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      std::string m_name;
      Expression m_value;

      SetVariableExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline SetVariableExpression::SetVariableExpression(
    std::string name, Expression value)
    : m_name(std::move(name)),
      m_value(std::move(value)) {}

  inline const std::string& SetVariableExpression::get_name() const {
    return m_name;
  }

  inline const Expression& SetVariableExpression::get_value() const {
    return m_value;
  }

  inline std::type_index SetVariableExpression::get_type() const {
    return m_value.get_type();
  }

  inline void SetVariableExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& SetVariableExpression::to_stream(
      std::ostream& out) const {
    return out << "(set " << m_name << ' ' << m_value << ')';
  }

  inline SetVariableExpression::SetVariableExpression()
    : m_value(ConstantExpression(false)) {}

  template<IsShuttle S>
  void SetVariableExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("value", m_value);
  }

  inline void ExpressionVisitor::visit(
      const SetVariableExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
