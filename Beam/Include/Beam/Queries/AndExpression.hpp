#ifndef BEAM_QUERIES_AND_EXPRESSION_HPP
#define BEAM_QUERIES_AND_EXPRESSION_HPP
#include <utility>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** Represents a logical and expression. */
  class AndExpression :
      public VirtualExpression, public CloneableMixin<AndExpression> {
    public:

      /**
       * Constructs an AndExpression.
       * @param lhs The left hand side of the Expression.
       * @param rhs The right hand side of the Expression.
       */
      AndExpression(Expression lhs, Expression rhs);

      /**
       * Copies an AndExpression.
       * @param expression The AndExpression to copy.
       */
      AndExpression(const AndExpression& expression) = default;

      /** Returns the left hand side of the Expression. */
      const Expression& GetLeftExpression() const;

      /** Returns the right hand side of the Expression. */
      const Expression& GetRightExpression() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      Expression m_left;
      Expression m_right;

      AndExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  /**
   * Makes an Expression that represents the logical and over a sequence of
   * sub-Expressions.
   * @param first An iterator to the first Expression.
   * @param last An iterator to one past the last Expression.
   * @return An Expression that represents the logical and over the sequence of
   *         sub-Expressions.
   */
  template<typename ForwardIterator>
  inline Expression MakeAndExpression(
      ForwardIterator first, ForwardIterator last) {
    if(first == last) {
      return ConstantExpression(false);
    }
    if((*first)->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
    if(first + 1 == last) {
      return *first;
    } else {
      auto right = MakeAndExpression(first + 1, last);
      return AndExpression(*first, right);
    }
  }

  inline AndExpression::AndExpression(Expression lhs, Expression rhs)
      : m_left(std::move(lhs)),
        m_right(std::move(rhs)) {
    if(m_left->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
    if(m_right->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
  }

  inline const Expression& AndExpression::GetLeftExpression() const {
    return m_left;
  }

  inline const Expression& AndExpression::GetRightExpression() const {
    return m_right;
  }

  inline const DataType& AndExpression::GetType() const {
    static auto value = DataType(BoolType::GetInstance());
    return value;
  }

  inline void AndExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& AndExpression::ToStream(std::ostream& out) const {
    return out << "(and " << GetLeftExpression() << " " <<
      GetRightExpression() << ")";
  }

  inline AndExpression::AndExpression()
    : m_left(ConstantExpression(false)),
      m_right(ConstantExpression(false)) {}

  template<typename Shuttler>
  void AndExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("left", m_left);
    shuttle.Shuttle("right", m_right);
    if(Serialization::IsReceiver<Shuttler>::value) {
      if(m_left->GetType()->GetNativeType() != typeid(bool)) {
        BOOST_THROW_EXCEPTION(
          Serialization::SerializationException("Incompatible types."));
      }
      if(m_right->GetType()->GetNativeType() != typeid(bool)) {
        BOOST_THROW_EXCEPTION(
          Serialization::SerializationException("Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::Visit(const AndExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
