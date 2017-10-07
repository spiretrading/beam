#ifndef BEAM_QUERIESGLOBALVARIABLEDECLARATIONEXPRESSION_HPP
#define BEAM_QUERIESGLOBALVARIABLEDECLARATIONEXPRESSION_HPP
#include <string>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class GlobalVariableDeclarationExpression
      \brief Declares a global variable and evaluates an expression.
   */
  class GlobalVariableDeclarationExpression : public VirtualExpression,
      public CloneableMixin<GlobalVariableDeclarationExpression> {
    public:

      //! Constructs a GlobalVariableDeclarationExpression.
      /*!
        \param name The name of the variable.
        \param initialValue The variable's initial value.
        \param body The body to evaluate.
      */
      GlobalVariableDeclarationExpression(const std::string& name,
        const Expression& initialValue, const Expression& body);

      //! Copies a GlobalVariableDeclarationExpression.
      /*!
        \param expression The GlobalVariableDeclarationExpression to copy.
      */
      GlobalVariableDeclarationExpression(
        const GlobalVariableDeclarationExpression& expression) = default;

      //! Returns the name of the variable.
      const std::string& GetName() const;

      //! Returns the variable's initial value.
      const Expression& GetInitialValue() const;

      //! Returns the body to evaluate.
      const Expression& GetBody() const;

      virtual const DataType& GetType() const;

      virtual void Apply(ExpressionVisitor& visitor) const;

    protected:
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      Expression m_initialValue;
      Expression m_body;

      GlobalVariableDeclarationExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline GlobalVariableDeclarationExpression::
      GlobalVariableDeclarationExpression(const std::string& name,
      const Expression& initialValue, const Expression& body)
      : m_name{name},
        m_initialValue{initialValue},
        m_body{body} {}

  inline const std::string& GlobalVariableDeclarationExpression::
      GetName() const {
    return m_name;
  }

  inline const Expression& GlobalVariableDeclarationExpression::
      GetInitialValue() const {
    return m_initialValue;
  }

  inline const Expression& GlobalVariableDeclarationExpression::
      GetBody() const {
    return m_body;
  }

  inline const DataType& GlobalVariableDeclarationExpression::GetType() const {
    return m_body->GetType();
  }

  inline void GlobalVariableDeclarationExpression::Apply(
      ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& GlobalVariableDeclarationExpression::ToStream(
      std::ostream& out) const {
    return out << "(global (" << m_name << " " << *m_initialValue << ") " <<
      *m_body << ")";
  }

  inline GlobalVariableDeclarationExpression::
      GlobalVariableDeclarationExpression()
      : m_initialValue{MakeConstantExpression(false)},
        m_body{MakeConstantExpression(false)} {}

  template<typename Shuttler>
  void GlobalVariableDeclarationExpression::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("initial_value", m_initialValue);
    shuttle.Shuttle("body", m_body);
  }

  inline void ExpressionVisitor::Visit(
      const GlobalVariableDeclarationExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}
}

#endif
