#ifndef BEAM_REDUCEEXPRESSION_HPP
#define BEAM_REDUCEEXPRESSION_HPP
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Queries/Value.hpp"

namespace Beam {
namespace Queries {

  /*! \class ReduceExpression
      \brief Consolidates a series of data into a singular value.
   */
  class ReduceExpression : public VirtualExpression,
      public CloneableMixin<ReduceExpression> {
    public:

      //! Constructs a ReduceExpression.
      /*!
        \param reduceExpression The expression used to perform the reduction.
        \param seriesExpression The expression to apply the reduction to.
        \param initialValue The initial value.
      */
      ReduceExpression(const Expression& reduceExpression,
        const Expression& seriesExpression, const Value& initialValue);

      //! Returns the reduce expression.
      const Expression& GetReduceExpression() const;

      //! Returns the series expression.
      const Expression& GetSeriesExpression() const;

      //! Returns the initial value.
      const Value& GetInitialValue() const;

      virtual const DataType& GetType() const;

      virtual void Apply(ExpressionVisitor& visitor) const;

    protected:
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend struct Serialization::DataShuttle;
      Expression m_reduceExpresssion;
      Expression m_seriesExpresssion;
      Value m_initialValue;

      ReduceExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline ReduceExpression::ReduceExpression(const Expression& reduceExpression,
      const Expression& seriesExpression, const Value& initialValue)
      : m_reduceExpresssion(reduceExpression),
        m_seriesExpresssion(seriesExpression),
        m_initialValue(initialValue) {
    if(m_initialValue->GetType() != m_reduceExpresssion->GetType()) {
      BOOST_THROW_EXCEPTION(TypeCompatibilityException());
    }
    if(m_seriesExpresssion->GetType() != m_reduceExpresssion->GetType()) {
      BOOST_THROW_EXCEPTION(TypeCompatibilityException());
    }
  }

  inline const Expression& ReduceExpression::GetReduceExpression() const {
    return m_reduceExpresssion;
  }

  inline const Expression& ReduceExpression::GetSeriesExpression() const {
    return m_seriesExpresssion;
  }

  inline const Value& ReduceExpression::GetInitialValue() const {
    return m_initialValue;
  }

  inline const DataType& ReduceExpression::GetType() const {
    return m_initialValue->GetType();
  }

  inline void ReduceExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& ReduceExpression::ToStream(std::ostream& out) const {
    return out << "(reduce " << *m_reduceExpresssion << " " <<
      *m_seriesExpresssion << " " << *m_initialValue << ")";
  }

  inline ReduceExpression::ReduceExpression()
      : m_reduceExpresssion(ConstantExpression(0)),
        m_seriesExpresssion(ConstantExpression(0)),
        m_initialValue(NativeValue(0)) {}

  template<typename Shuttler>
  void ReduceExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("reduce_expression", m_reduceExpresssion);
    shuttle.Shuttle("series_expression", m_seriesExpresssion);
    shuttle.Shuttle("initial_value", m_initialValue);
    if(Serialization::IsReceiver<Shuttler>::value) {
      if(m_initialValue->GetType()->GetNativeType() !=
          m_reduceExpresssion->GetType()->GetNativeType()) {
        BOOST_THROW_EXCEPTION(Serialization::SerializationException(
          "Incompatible types."));
      }
      if(m_seriesExpresssion->GetType()->GetNativeType() !=
          m_reduceExpresssion->GetType()->GetNativeType()) {
        BOOST_THROW_EXCEPTION(Serialization::SerializationException(
          "Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::Visit(const ReduceExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}
}

#endif
