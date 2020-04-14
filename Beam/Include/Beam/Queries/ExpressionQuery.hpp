#ifndef BEAM_EXPRESSIONQUERY_HPP
#define BEAM_EXPRESSIONQUERY_HPP
#include <ostream>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class ExpressionQuery
      \brief Represents a query that applies an Expression to retrieved values.
   */
  class ExpressionQuery {
    public:

      /*! \enum UpdatePolicy
          \brief Indicates when updated values should be sent.
       */
      enum class UpdatePolicy {

        //! Update on every value.
        ALL,

        //! Update when a change in value is produced.
        CHANGE
      };

      //! Constructs an ExpressionQuery returning the value unmodified.
      ExpressionQuery();

      //! Constructs an ExpressionQuery with a specified Expression.
      /*!
        \param expression The Expression to apply.
      */
      ExpressionQuery(const Expression& expression);

      //! Returns the UpdatePolicy.
      UpdatePolicy GetUpdatePolicy() const;

      //! Sets the UpdatePolicy.
      void SetUpdatePolicy(UpdatePolicy policy);

      //! Returns the Expression.
      const Expression& GetExpression() const;

      //! Sets the Expression to apply.
      /*!
        \param expression The Expression to apply.
      */
      void SetExpression(const Expression& expression);

    private:
      friend struct Serialization::Shuttle<ExpressionQuery>;
      UpdatePolicy m_updatePolicy;
      Expression m_expression;
  };

  inline std::ostream& operator <<(std::ostream& out,
      ExpressionQuery::UpdatePolicy value) {
    if(value == ExpressionQuery::UpdatePolicy::ALL) {
      return out << "ALL";
    } else if(value == ExpressionQuery::UpdatePolicy::CHANGE) {
      return out << "CHANGE";
    }
    return out << "NONE";
  }

  inline std::ostream& operator <<(std::ostream& out,
      const ExpressionQuery& query) {
    return out << "(" << query.GetUpdatePolicy() << " " <<
      query.GetExpression() << ")";
  }

  inline ExpressionQuery::ExpressionQuery()
    : m_updatePolicy(UpdatePolicy::ALL),
      m_expression(ConstantExpression(true)) {}

  inline ExpressionQuery::ExpressionQuery(const Expression& expression)
      : m_updatePolicy{UpdatePolicy::ALL},
        m_expression{expression} {}

  inline ExpressionQuery::UpdatePolicy ExpressionQuery::
      GetUpdatePolicy() const {
    return m_updatePolicy;
  }

  inline void ExpressionQuery::SetUpdatePolicy(UpdatePolicy policy) {
    m_updatePolicy = policy;
  }

  inline const Expression& ExpressionQuery::GetExpression() const {
    return m_expression;
  }

  inline void ExpressionQuery::SetExpression(const Expression& expression) {
    m_expression = expression;
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Queries::ExpressionQuery> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::ExpressionQuery& value,
        unsigned int version) {
      shuttle.Shuttle("update_policy", value.m_updatePolicy);
      shuttle.Shuttle("expression", value.m_expression);
    }
  };
}
}

#endif
