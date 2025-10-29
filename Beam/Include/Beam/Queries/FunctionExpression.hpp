#ifndef BEAM_QUERIES_FUNCTION_EXPRESSION_HPP
#define BEAM_QUERIES_FUNCTION_EXPRESSION_HPP
#include <string>
#include <utility>
#include <vector>
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"

namespace Beam {

  /** An Expression representing a native function. */
  class FunctionExpression : public VirtualExpression {
    public:

      /**
       * Constructs a FunctionExpression.
       * @param name The name of the function.
       * @param type The type that this function evaluates to.
       * @param parameters The function's parameters.
       */
      FunctionExpression(std::string name, std::type_index type,
        std::vector<Expression> parameters);

      /** Returns the name of the function. */
      const std::string& get_name() const;

      /** Returns the function's parameters. */
      const std::vector<Expression>& get_parameters() const;

      std::type_index get_type() const override;
      void apply(ExpressionVisitor& visitor) const override;

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      friend struct DataShuttle;
      std::string m_name;
      std::type_index m_type;
      std::vector<Expression> m_parameters;

      FunctionExpression();
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  inline FunctionExpression::FunctionExpression(
    std::string name, std::type_index type, std::vector<Expression> parameters)
    : m_name(std::move(name)),
      m_type(std::move(type)),
      m_parameters(std::move(parameters)) {}

  inline const std::string& FunctionExpression::get_name() const {
    return m_name;
  }

  inline const std::vector<Expression>&
      FunctionExpression::get_parameters() const {
    return m_parameters;
  }

  inline std::type_index FunctionExpression::get_type() const {
    return m_type;
  }

  inline void FunctionExpression::apply(ExpressionVisitor& visitor) const {
    visitor.visit(*this);
  }

  inline std::ostream& FunctionExpression::to_stream(std::ostream& out) const {
    out << '(' << m_name;
    for(auto& parameter : m_parameters) {
      out << ' ' << parameter;
    }
    return out << ')';
  }

  inline FunctionExpression::FunctionExpression()
    : FunctionExpression("", typeid(bool), {}) {}

  template<IsShuttle S>
  void FunctionExpression::shuttle(S& shuttle, unsigned int version) {
    VirtualExpression::shuttle(shuttle, version);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("type", m_type);
    shuttle.shuttle("parameters", m_parameters);
  }

  inline void ExpressionVisitor::visit(const FunctionExpression& expression) {
    visit(static_cast<const VirtualExpression&>(expression));
  }
}

#endif
