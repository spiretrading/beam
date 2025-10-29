#ifndef BEAM_EXPRESSION_QUERY_HPP
#define BEAM_EXPRESSION_QUERY_HPP
#include <ostream>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleUniquePtr.hpp"

namespace Beam {

  /** Represents a query that applies an Expression to retrieved values. */
  class ExpressionQuery {
    public:

      /** Indicates when updated values should be sent. */
      enum class UpdatePolicy {

        /** Update on every value. */
        ALL,

        /** Update when a change in value is produced. */
        CHANGE
      };

      /** Constructs an ExpressionQuery returning the value unmodified. */
      ExpressionQuery();

      /**
       * Constructs an ExpressionQuery with a specified Expression.
       * @param expression The Expression to apply.
       */
      explicit ExpressionQuery(const Expression& expression);

      /** Returns the UpdatePolicy. */
      UpdatePolicy get_update_policy() const;

      /** Sets the UpdatePolicy. */
      void set_update_policy(UpdatePolicy policy);

      /** Returns the Expression. */
      const Expression& get_expression() const;

      /**
       * Sets the Expression to apply.
       * @param expression The Expression to apply.
       */
      void set_expression(const Expression& expression);

    private:
      friend struct Shuttle<ExpressionQuery>;
      UpdatePolicy m_update_policy;
      Expression m_expression;
  };

  inline std::ostream& operator <<(
      std::ostream& out, ExpressionQuery::UpdatePolicy value) {
    if(value == ExpressionQuery::UpdatePolicy::ALL) {
      return out << "ALL";
    } else if(value == ExpressionQuery::UpdatePolicy::CHANGE) {
      return out << "CHANGE";
    }
    return out << "NONE";
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ExpressionQuery& query) {
    return out << '(' << query.get_update_policy() << ' ' <<
      query.get_expression() << ')';
  }

  inline ExpressionQuery::ExpressionQuery()
    : m_update_policy(UpdatePolicy::ALL),
      m_expression(ConstantExpression(true)) {}

  inline ExpressionQuery::ExpressionQuery(const Expression& expression)
    : m_update_policy(UpdatePolicy::ALL),
      m_expression(expression) {}

  inline ExpressionQuery::UpdatePolicy
      ExpressionQuery::get_update_policy() const {
    return m_update_policy;
  }

  inline void ExpressionQuery::set_update_policy(UpdatePolicy policy) {
    m_update_policy = policy;
  }

  inline const Expression& ExpressionQuery::get_expression() const {
    return m_expression;
  }

  inline void ExpressionQuery::set_expression(const Expression& expression) {
    m_expression = expression;
  }

  template<>
  struct Shuttle<ExpressionQuery> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, ExpressionQuery& value, unsigned int version) const {
      shuttle.shuttle("update_policy", value.m_update_policy);
      shuttle.shuttle("expression", value.m_expression);
    }
  };
}

#endif
