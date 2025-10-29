#ifndef BEAM_QUERIES_AND_EXPRESSION_HPP
#define BEAM_QUERIES_AND_EXPRESSION_HPP
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"

namespace Beam {

  /** Represents a logical and expression. */
  class AndExpression : public VirtualExpression {
    public:

      /**
       * Constructs an AndExpression.
       * @param lhs The left hand side of the Expression.
       * @param rhs The right hand side of the Expression.
       */
      AndExpression(Expression lhs, Expression rhs);

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

      AndExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  /**
   * Constructs an AndExpression.
   * @param left The left hand side of the Expression.
   * @param right The right hand side of the Expression.
   * @return An AndExpression.
   */
  inline AndExpression operator &&(
      const Expression& left, const Expression& right) {
    return AndExpression(left, right);
  }

  /**
   * Makes an Expression that represents the logical and over a sequence of
   * sub-Expressions.
   * @param first An iterator to the first Expression.
   * @param last An iterator to one past the last Expression.
   * @return An Expression that represents the logical and over the sequence of
   *         sub-Expressions.
   */
  template<std::forward_iterator I>
  Expression conjunction(I first, I last) {
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
      return AndExpression(*first, conjunction(first + 1, last));
    }
  }

  inline AndExpression::AndExpression(Expression lhs, Expression rhs)
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

  inline const Expression& AndExpression::get_left() const {
    return m_left;
  }

  inline const Expression& AndExpression::get_right() const {
    return m_right;
  }

  inline std::type_index AndExpression::get_type() const {
    return typeid(bool);
  }

  inline void AndExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& AndExpression::to_stream(std::ostream& out) const {
    return out << "(and " << get_left() << ' ' << get_right() << ')';
  }

  inline AndExpression::AndExpression()
    : m_left(ConstantExpression(false)),
      m_right(ConstantExpression(false)) {}

  template<IsShuttle S>
  void AndExpression::shuttle(S& shuttle, unsigned int version) {
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

  inline void ExpressionVisitor::visit(const AndExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
