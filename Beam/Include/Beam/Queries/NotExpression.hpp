#ifndef BEAM_QUERIES_NOT_EXPRESSION_HPP
#define BEAM_QUERIES_NOT_EXPRESSION_HPP
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

  /** Represents a logical not expression. */
  class NotExpression :
      public VirtualExpression, public CloneableMixin<NotExpression> {
    public:

      /**
       * Constructs a NotExpression.
       * @param operand The operand to evaluate.
       */
      NotExpression(Expression operand);

      /**
       * Copies a NotExpression.
       * @param expression The NotExpression to copy.
       */
      NotExpression(const NotExpression& expression) = default;

      /** Returns the operand. */
      const Expression& GetOperand() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      Expression m_operand;

      NotExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline NotExpression::NotExpression(Expression operand)
      : m_operand(std::move(operand)) {
    if(m_operand->GetType()->GetNativeType() != typeid(bool)) {
      BOOST_THROW_EXCEPTION(
        TypeCompatibilityException("Expression must be bool."));
    }
  }

  inline const Expression& NotExpression::GetOperand() const {
    return m_operand;
  }

  inline const DataType& NotExpression::GetType() const {
    static auto value = DataType(BoolType::GetInstance());
    return value;
  }

  inline void NotExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& NotExpression::ToStream(std::ostream& out) const {
    return out << "(not " << GetOperand() << ")";
  }

  inline NotExpression::NotExpression()
    : m_operand(ConstantExpression(false)) {}

  template<typename Shuttler>
  void NotExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("operand", m_operand);
    if(Serialization::IsReceiver<Shuttler>::value) {
      if(m_operand->GetType()->GetNativeType() != typeid(bool)) {
        BOOST_THROW_EXCEPTION(
          Serialization::SerializationException("Incompatible types."));
      }
    }
  }

  inline void ExpressionVisitor::Visit(const NotExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
