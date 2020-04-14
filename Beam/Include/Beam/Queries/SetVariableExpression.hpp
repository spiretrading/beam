#ifndef BEAM_QUERIESSETVARIABLEEXPRESSION_HPP
#define BEAM_QUERIESSETVARIABLEEXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class SetVariableExpression
      \brief Sets the value of a variable.
   */
  class SetVariableExpression : public VirtualExpression,
      public CloneableMixin<SetVariableExpression> {
    public:

      //! Constructs a SetVariableExpression.
      /*!
        \param name The name of the variable to set.
        \param value The value to assign to the variable.
      */
      SetVariableExpression(const std::string& name, const Expression& value);

      //! Copies a SetVariableExpression.
      /*!
        \param expression The SetVariableExpression to copy.
      */
      SetVariableExpression(const SetVariableExpression& expression) = default;

      //! Returns the name of the variable to set.
      const std::string& GetName() const;

      //! Returns the value to set the variable to.
      const Expression& GetValue() const;

      virtual const DataType& GetType() const;

      virtual void Apply(ExpressionVisitor& visitor) const;

    protected:
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      Expression m_value;

      SetVariableExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline SetVariableExpression::SetVariableExpression(const std::string& name,
      const Expression& value)
      : m_name(name),
        m_value(value) {}

  inline const std::string& SetVariableExpression::GetName() const {
    return m_name;
  }

  inline const Expression& SetVariableExpression::GetValue() const {
    return m_value;
  }

  inline const DataType& SetVariableExpression::GetType() const {
    return m_value->GetType();
  }

  inline void SetVariableExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& SetVariableExpression::ToStream(
      std::ostream& out) const {
    return out << "(set " << m_name << " " << *m_value << ")";
  }

  inline SetVariableExpression::SetVariableExpression()
      : m_value(ConstantExpression(false)) {}

  template<typename Shuttler>
  void SetVariableExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("value", m_value);
  }

  inline void ExpressionVisitor::Visit(
      const SetVariableExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}
}

#endif
