#ifndef BEAM_QUERIES_VARIABLE_EXPRESSION_HPP
#define BEAM_QUERIES_VARIABLE_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Evaluates to the current value of a variable. */
  class VariableExpression : public VirtualExpression {
    public:

      /**
       * Constructs a VariableExpression.
       * @param name The name of the variable.
       * @param data_type The variable's data type.
       */
      VariableExpression(std::string name, std::type_index data_type);

      /** Returns the name of the variable. */
      const std::string& get_name() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      std::string m_name;
      std::type_index m_data_type;

      VariableExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline VariableExpression::VariableExpression(
    std::string name, std::type_index data_type)
    : m_name(std::move(name)),
      m_data_type(data_type) {}

  inline const std::string& VariableExpression::get_name() const {
    return m_name;
  }

  inline std::type_index VariableExpression::get_type() const {
    return m_data_type;
  }

  inline void VariableExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& VariableExpression::to_stream(std::ostream& out) const {
    return out << m_name;
  }

  inline VariableExpression::VariableExpression()
    : m_data_type(typeid(bool)) {}

  template<IsShuttle S>
  void VariableExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("data_type", m_data_type);
  }

  inline void ExpressionVisitor::visit(const VariableExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
