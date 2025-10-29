#ifndef BEAM_QUERIES_OR_EXPRESSION_HPP
#define BEAM_QUERIES_OR_EXPRESSION_HPP
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Represents a logical or expression. */
  class OrExpression : public VirtualExpression {
    public:

      /**
       * Constructs an OrExpression.
       * @param left The left hand side of the Expression.
       * @param right The right hand side of the Expression.
       */
      OrExpression(Expression left, Expression right);

      /** Returns the left hand side of the Expression. */
      const Expression& get_left() const;

      /** Returns the right hand side of the Expression. */
      const Expression& get_right() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      Expression m_left;
      Expression m_right;

      OrExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  /**
   * Constructs an OrExpression.
   * @param left The left hand side of the Expression.
   * @param right The right hand side of the Expression.
   * @return An OrExpression.
   */
  inline OrExpression operator ||(
      const Expression& left, const Expression& right) {
    return OrExpression(left, right);
  }

  /**
   * Makes an Expression that represents the logical or over a sequence of
   * sub-expressions.
   * @param first An iterator to the first Expression.
   * @param last An iterator to one past the last Expression.
   * @return An Expression that represents the logical or over the sequence of
   *         sub-expressions.
   */
  template<std::forward_iterator I>
  Expression disjunction(I first, I last) {
    if(first == last) {
      return ConstantExpression(false);
    }
    if(first->get_type() != typeid(bool)) {
      boost::throw_with_location(
        TypeCompatibilityException("Expression must be bool."));
    }
    if(first + 1 == last) {
      return *first;
    } else {
      return OrExpression(*first, disjunction(first + 1, last));
    }
  }

  inline OrExpression::OrExpression(Expression lhs, Expression rhs)
      : m_left(std::move(lhs)),
        m_right(std::move(rhs)) {
    if(m_left.get_type() != typeid(bool)) {
      boost::throw_with_location(
        TypeCompatibilityException("Expression must be bool."));
    }
    if(m_right.get_type() != typeid(bool)) {
      boost::throw_with_location(
        TypeCompatibilityException("Expression must be bool."));
    }
  }

  inline const Expression& OrExpression::get_left() const {
    return m_left;
  }

  inline const Expression& OrExpression::get_right() const {
    return m_right;
  }

  inline std::type_index OrExpression::get_type() const {
    return typeid(bool);
  }

  inline void OrExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& OrExpression::to_stream(std::ostream& out) const {
    return out << "(or " << get_left() << " " << get_right() << ')';
  }

  inline OrExpression::OrExpression()
    : m_left(ConstantExpression(false)),
      m_right(ConstantExpression(false)) {}

  template<IsShuttle S>
  void OrExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("left", m_left);
    shuttle.shuttle("right", m_right);
    if(IsReceiver<S>) {
      if(m_left.get_type() != typeid(bool)) {
        boost::throw_with_location(
          SerializationException("Incompatible types."));
      }
      if(m_right.get_type() != typeid(bool)) {
        boost::throw_with_location(
          SerializationException("Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::visit(const OrExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
