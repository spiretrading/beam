#ifndef BEAM_QUERIES_NOT_EXPRESSION_HPP
#define BEAM_QUERIES_NOT_EXPRESSION_HPP
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Represents a logical not expression. */
  class NotExpression : public VirtualExpression {
    public:

      /**
       * Constructs a NotExpression.
       * @param operand The operand to evaluate.
       */
      explicit NotExpression(Expression operand);

      /** Returns the operand. */
      const Expression& get_operand() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      Expression m_operand;

      NotExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  /**
   * Constructs a NotExpression.
   * @param operand The expression to negate.
   * @return A NotExpression negating its operand.
   */
  inline NotExpression operator !(const Expression& operand) {
    return NotExpression(operand);
  }

  inline NotExpression::NotExpression(Expression operand)
      : m_operand(std::move(operand)) {
    if(m_operand.get_type() != typeid(bool)) {
      boost::throw_with_location(
        TypeCompatibilityException("Expression must be bool."));
    }
  }

  inline const Expression& NotExpression::get_operand() const {
    return m_operand;
  }

  inline std::type_index NotExpression::get_type() const {
    return typeid(bool);
  }

  inline void NotExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& NotExpression::to_stream(std::ostream& out) const {
    return out << "(not " << get_operand() << ')';
  }

  inline NotExpression::NotExpression()
    : m_operand(ConstantExpression(false)) {}

  template<IsShuttle S>
  void NotExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("operand", m_operand);
    if(IsReceiver<S>) {
      if(m_operand.get_type() != typeid(bool)) {
        boost::throw_with_location(
          SerializationException("Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::visit(const NotExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
