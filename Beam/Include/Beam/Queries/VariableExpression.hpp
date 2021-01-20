#ifndef BEAM_QUERIES_VARIABLE_EXPRESSION_HPP
#define BEAM_QUERIES_VARIABLE_EXPRESSION_HPP
#include <string>
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam::Queries {

  /** Evaluates to the current value of a variable. */
  class VariableExpression :
      public VirtualExpression, public CloneableMixin<VariableExpression> {
    public:

      /**
       * Constructs a VariableExpression.
       * @param name The name of the variable.
       * @param dataType The variable's data type.
       */
      VariableExpression(std::string name, DataType dataType);

      /**
       * Copies a VariableExpression.
       * @param expression The VariableExpression to copy.
       */
      VariableExpression(const VariableExpression& expression) = default;

      /** Returns the name of the variable. */
      const std::string& GetName() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      DataType m_dataType;

      VariableExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline VariableExpression::VariableExpression(std::string name,
    DataType dataType)
    : m_name(std::move(name)),
      m_dataType(std::move(dataType)) {}

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

#endif
