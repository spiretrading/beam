#ifndef BEAM_MEMBER_ACCESS_EXPRESSION_HPP
#define BEAM_MEMBER_ACCESS_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** An Expression used to access an object's data member. */
  class MemberAccessExpression : public VirtualExpression,
      public CloneableMixin<MemberAccessExpression> {
    public:

      /**
       * Constructs a MemberAccessExpression.
       * @param name The name of the member to access.
       * @param type The member's DataType.
       * @param expression The Expression to access.
       */
      MemberAccessExpression(std::string name, DataType type,
        Expression expression);

      /** Returns name of the member to access. */
      const std::string& GetName() const;

      /** Returns the Expression to access. */
      const Expression& GetExpression() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      DataType m_type;
      Expression m_expression;

      MemberAccessExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline MemberAccessExpression::MemberAccessExpression(std::string name,
    DataType type, Expression expression)
    : m_name(std::move(name)),
      m_type(std::move(type)),
      m_expression(std::move(expression)) {}

  inline const std::string& MemberAccessExpression::GetName() const {
    return m_name;
  }

  inline const Expression& MemberAccessExpression::GetExpression() const {
    return m_expression;
  }

  inline const DataType& MemberAccessExpression::GetType() const {
    return m_type;
  }

  inline void MemberAccessExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& MemberAccessExpression::ToStream(
      std::ostream& out) const {
    return out << *m_expression << "." << m_name;
  }

  inline MemberAccessExpression::MemberAccessExpression()
    : MemberAccessExpression("", BoolType(), ConstantExpression(false)) {}

  template<typename Shuttler>
  void MemberAccessExpression::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("type", m_type);
    shuttle.Shuttle("expression", m_expression);
  }

  inline void ExpressionVisitor::Visit(
      const MemberAccessExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
