#ifndef BEAM_MEMBER_ACCESS_EXPRESSION_HPP
#define BEAM_MEMBER_ACCESS_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** An Expression used to access an object's data member. */
  class MemberAccessExpression : public VirtualExpression {
    public:

      /**
       * Constructs a MemberAccessExpression.
       * @param name The name of the member to access.
       * @param type The member's DataType.
       * @param expression The Expression to access.
       */
      MemberAccessExpression(
        std::string name, std::type_index type, Expression expression);

      /** Returns name of the member to access. */
      const std::string& get_name() const;

      /** Returns the Expression to access. */
      const Expression& get_expression() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      std::string m_name;
      std::type_index m_type;
      Expression m_expression;

      MemberAccessExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline MemberAccessExpression::MemberAccessExpression(
    std::string name, std::type_index type, Expression expression)
    : m_name(std::move(name)),
      m_type(std::move(type)),
      m_expression(std::move(expression)) {}

  inline const std::string& MemberAccessExpression::get_name() const {
    return m_name;
  }

  inline const Expression& MemberAccessExpression::get_expression() const {
    return m_expression;
  }

  inline std::type_index MemberAccessExpression::get_type() const {
    return m_type;
  }

  inline void MemberAccessExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& MemberAccessExpression::to_stream(
      std::ostream& out) const {
    return out << m_expression << '.' << m_name;
  }

  inline MemberAccessExpression::MemberAccessExpression()
    : MemberAccessExpression("", typeid(bool), ConstantExpression(false)) {}

  template<IsShuttle S>
  void MemberAccessExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("type", m_type);
    shuttle.shuttle("expression", m_expression);
  }

  inline void ExpressionVisitor::visit(
      const MemberAccessExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
