#ifndef BEAM_PARAMETER_EXPRESSION_HPP
#define BEAM_PARAMETER_EXPRESSION_HPP
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"

namespace Beam {

  /** Represents a variable/parameter used in an Expression. */
  class ParameterExpression : public VirtualExpression {
    public:

      /**
       * Constructs a ParameterExpression.
       * @param index The parameter's index.
       * @param type The parameter's type.
       */
      ParameterExpression(int index, std::type_index type);

      /** Returns the parameter's index. */
      int get_index() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct Beam::DataShuttle;
      int m_index;
      std::type_index m_type;

      ParameterExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline ParameterExpression::ParameterExpression(
    int index, std::type_index type)
    : m_index(index),
      m_type(type) {}

  inline int ParameterExpression::get_index() const {
    return m_index;
  }

  inline std::type_index ParameterExpression::get_type() const {
    return m_type;
  }

  inline void ParameterExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& ParameterExpression::to_stream(std::ostream& out) const {
    return out << "(parameter " << m_index << ')';
  }

  inline ParameterExpression::ParameterExpression()
    : ParameterExpression(0, typeid(bool)) {}

  template<IsShuttle S>
  void ParameterExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("index", m_index);
    shuttle.shuttle("type", m_type);
  }

  inline void ExpressionVisitor::visit(const ParameterExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
