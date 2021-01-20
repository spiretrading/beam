#ifndef BEAM_QUERIES_FUNCTION_EXPRESSION_HPP
#define BEAM_QUERIES_FUNCTION_EXPRESSION_HPP
#include <string>
#include <utility>
#include <vector>
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"

namespace Beam::Queries {

  /** An Expression representing a native function. */
  class FunctionExpression : public VirtualExpression,
      public CloneableMixin<FunctionExpression> {
    public:

      /**
       * Constructs a FunctionExpression.
       * @param name The name of the function.
       * @param type The type that this function evaluates to.
       * @param parameters The function's parameters.
       */
      FunctionExpression(std::string name, DataType type,
        std::vector<Expression> parameters);

      /**
       * Copies a FunctionExpression.
       * @param expression The FunctionExpression to copy.
       */
      FunctionExpression(const FunctionExpression& expression) = default;

      /** Returns the name of the function. */
      const std::string& GetName() const;

      /** Returns the function's parameters. */
      const std::vector<Expression>& GetParameters() const;

      const DataType& GetType() const override;

      void Apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      friend struct Serialization::DataShuttle;
      std::string m_name;
      DataType m_type;
      std::vector<Expression> m_parameters;

      FunctionExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline FunctionExpression::FunctionExpression(std::string name, DataType type,
    std::vector<Expression> parameters)
    : m_name(std::move(name)),
      m_type(std::move(type)),
      m_parameters(std::move(parameters)) {}

  inline const std::string& FunctionExpression::GetName() const {
    return m_name;
  }

  inline const std::vector<Expression>&
      FunctionExpression::GetParameters() const {
    return m_parameters;
  }

  inline const DataType& FunctionExpression::GetType() const {
    return m_type;
  }

  inline void FunctionExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& FunctionExpression::ToStream(std::ostream& out) const {
    out << "(" << m_name;
    for(auto& parameter : m_parameters) {
      out << " " << *parameter;
    }
    return out << ")";
  }

  inline FunctionExpression::FunctionExpression()
    : FunctionExpression("", BoolType(), {}) {}

  template<typename Shuttler>
  void FunctionExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("type", m_type);
    shuttle.Shuttle("parameters", m_parameters);
  }

  inline void ExpressionVisitor::Visit(const FunctionExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
