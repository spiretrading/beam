#ifndef BEAM_QUERIES_CONSTANT_EXPRESSION_HPP
#define BEAM_QUERIES_CONSTANT_EXPRESSION_HPP
#include <utility>
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Value.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** An Expression that evaluates to a constant. */
  class ConstantExpression : public VirtualExpression {
    public:

      /**
       * Constructs a ConstantExpression representing a value.
       * @param value The value to evaluate to.
       */
      template<DisableCopy<ConstantExpression> T>
      ConstantExpression(T&& value);

      /** Returns the value to evaluate to. */
      const Value& get_value() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      Value m_value;

      ConstantExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<DisableCopy<ConstantExpression> T>
  ConstantExpression::ConstantExpression(T&& value)
    : m_value(std::forward<T>(value)) {}

  inline const Value& ConstantExpression::get_value() const {
    return m_value;
  }

  inline std::type_index ConstantExpression::get_type() const {
    return m_value.get_type();
  }

  inline void ConstantExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& ConstantExpression::to_stream(std::ostream& out) const {
    return out << m_value;
  }

  inline ConstantExpression::ConstantExpression()
    : m_value(0) {}

  template<IsShuttle S>
  void ConstantExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("value", m_value);
  }

  inline void ExpressionVisitor::visit(const ConstantExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
