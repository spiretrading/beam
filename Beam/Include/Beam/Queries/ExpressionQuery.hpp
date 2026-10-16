#ifndef BEAM_EXPRESSION_QUERY_HPP
#define BEAM_EXPRESSION_QUERY_HPP
#include <format>
#include <ostream>
#include <string_view>
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

template<>
class std::formatter<Beam::ExpressionQuery::UpdatePolicy> :
    public std::formatter<std::string_view> {
  public:
    auto format(
        Beam::ExpressionQuery::UpdatePolicy value, auto& context) const {
      auto name = [&] {
        if(value == Beam::ExpressionQuery::UpdatePolicy::ALL) {
          return std::string_view("ALL");
        } else if(value == Beam::ExpressionQuery::UpdatePolicy::CHANGE) {
          return std::string_view("CHANGE");
        }
        return std::string_view("NONE");
      }();
      return std::formatter<std::string_view>::format(name, context);
    }
};

template<>
class std::formatter<Beam::ExpressionQuery> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Beam::ExpressionQuery& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {})", value.get_update_policy(),
          value.get_expression()), context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& out, ExpressionQuery::UpdatePolicy value) {
    return out << std::format(out.getloc(), "{}", value);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const ExpressionQuery& query) {
    return out << std::format(out.getloc(), "{}", query);
  }
}

#endif
