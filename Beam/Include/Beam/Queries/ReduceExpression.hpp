#ifndef BEAM_REDUCE_EXPRESSION_HPP
#define BEAM_REDUCE_EXPRESSION_HPP
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Queries/Value.hpp"

namespace Beam {

  /** Consolidates a series of data into a singular value. */
  class ReduceExpression : public VirtualExpression {
    public:

      /**
       * Constructs a ReduceExpression.
       * @param reducer The expression used to perform the reduction.
       * @param series The expression to apply the reduction to.
       * @param initial_value The initial value.
       */
      ReduceExpression(
        Expression reducer, Expression series, Value initial_value);

      /** Returns the reduce expression. */
      const Expression& get_reducer() const;

      /** Returns the series expression. */
      const Expression& get_series() const;

      /** Returns the initial value. */
      const Value& get_initial_value() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      Expression m_reducer;
      Expression m_series;
      Value m_initial_value;

      ReduceExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline ReduceExpression::ReduceExpression(
      Expression reducer, Expression series, Value initial_value)
      : m_reducer(std::move(reducer)),
        m_series(std::move(series)),
        m_initial_value(std::move(initial_value)) {
    if(m_initial_value.get_type() != m_reducer.get_type()) {
      boost::throw_with_location(TypeCompatibilityException());
    }
    if(m_series.get_type() != m_reducer.get_type()) {
      boost::throw_with_location(TypeCompatibilityException());
    }
  }

  inline const Expression& ReduceExpression::get_reducer() const {
    return m_reducer;
  }

  inline const Expression& ReduceExpression::get_series() const {
    return m_series;
  }

  inline const Value& ReduceExpression::get_initial_value() const {
    return m_initial_value;
  }

  inline std::type_index ReduceExpression::get_type() const {
    return m_initial_value.get_type();
  }

  inline void ReduceExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& ReduceExpression::to_stream(std::ostream& out) const {
    return out << "(reduce " << m_reducer << ' ' << m_series << ' ' <<
      m_initial_value << ')';
  }

  inline ReduceExpression::ReduceExpression()
    : m_reducer(ConstantExpression(0)),
      m_series(ConstantExpression(0)),
      m_initial_value(0) {}

  template<IsShuttle S>
  void ReduceExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("reducer", m_reducer);
    shuttle.shuttle("series", m_series);
    shuttle.shuttle("initial_value", m_initial_value);
    if(IsReceiver<S>) {
      if(m_initial_value.get_type() != m_reducer.get_type()) {
        boost::throw_with_location(
          SerializationException("Incompatible types."));
      }
      if(m_series.get_type() != m_reducer.get_type()) {
        boost::throw_with_location(
          SerializationException("Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::visit(const ReduceExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
