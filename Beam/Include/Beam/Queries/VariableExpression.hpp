#ifndef BEAM_QUERIESVARIABLEEXPRESSION_HPP
#define BEAM_QUERIESVARIABLEEXPRESSION_HPP
#include <string>
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class VariableExpression
      \brief Evaluates to the current value of a variable.
   */
  class VariableExpression : public VirtualExpression,
      public CloneableMixin<VariableExpression> {
    public:

      //! Constructs a VariableExpression.
      /*!
        \param name The name of the variable.
        \param dataType The variable's data type.
      */
      VariableExpression(const std::string& name, const DataType& dataType);

      //! Copies a VariableExpression.
      /*!
        \param expression The VariableExpression to copy.
      */
      VariableExpression(const VariableExpression& expression) = default;

      //! Returns the name of the variable.
      const std::string& GetName() const;

      virtual const DataType& GetType() const;

      virtual void Apply(ExpressionVisitor& visitor) const;

    protected:
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      DataType m_dataType;

      VariableExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline VariableExpression::VariableExpression(const std::string& name,
      const DataType& dataType)
      : m_name(name),
        m_dataType(dataType) {}

  inline const std::string& VariableExpression::GetName() const {
    return m_name;
  }

  inline const DataType& VariableExpression::GetType() const {
    return m_dataType;
  }

  inline void VariableExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& VariableExpression::ToStream(std::ostream& out) const {
    return out << m_name;
  }

  inline VariableExpression::VariableExpression()
      : m_dataType(BoolType()) {}

  template<typename Shuttler>
  void VariableExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("data_type", m_dataType);
  }

  inline void ExpressionVisitor::Visit(const VariableExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}
}

#endif
