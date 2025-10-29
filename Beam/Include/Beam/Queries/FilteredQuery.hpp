#ifndef BEAM_FILTERED_QUERY_HPP
#define BEAM_FILTERED_QUERY_HPP
#include <ostream>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Evaluator.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"
#include "Beam/Queries/Value.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam {

  /** Filters what values should be returned in a Query. */
  class FilteredQuery {
    public:

      /** Constructs a FilteredQuery that returns all values. */
      FilteredQuery();

      /**
       * Constructs a FilteredQuery with a specified filter.
       * @param filter The Expression used as the filter.
       */
      explicit FilteredQuery(Expression filter);

      /** Returns the filter. */
      const Expression& get_filter() const;

      /** Sets the filter. */
      void set_filter(const Expression& filter);

    private:
      friend struct Shuttle<FilteredQuery>;
      Expression m_filter;
  };

  /**
   * Uses an Evaluator to test whether a value passes a filter.
   * @param evaluator The Evaluator used as the filter.
   * @param value The value to filter.
   * @return <code>true</code> iff the <i>value</i> passes the filter.
   */
  template<typename T>
  bool test_filter(Evaluator& evaluator, const T& value) {
    try {
      return evaluator.eval<bool>(value);
    } catch(const std::exception&) {
      return false;
    }
  }

  inline std::ostream& operator <<(
      std::ostream& out, const FilteredQuery& query) {
    return out << query.get_filter();
  }

  inline FilteredQuery::FilteredQuery()
    : FilteredQuery(ConstantExpression(true)) {}

  inline FilteredQuery::FilteredQuery(Expression filter)
      : m_filter(std::move(filter)) {
    if(m_filter.get_type() != typeid(bool)) {
      boost::throw_with_location(
        TypeCompatibilityException("Filter is not boolean."));
    }
  }

  inline const Expression& FilteredQuery::get_filter() const {
    return m_filter;
  }

  inline void FilteredQuery::set_filter(const Expression& filter) {
    if(filter.get_type() != typeid(bool)) {
      boost::throw_with_location(
        TypeCompatibilityException("Filter is not boolean."));
    }
    m_filter = filter;
  }

  template<>
  struct Shuttle<FilteredQuery> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, FilteredQuery& value, unsigned int version) const {
      shuttle.shuttle("filter", value.m_filter);
      if(IsReceiver<S>) {
        if(value.m_filter.get_type() != typeid(bool)) {
          value.m_filter = ConstantExpression(false);
          boost::throw_with_location(
            SerializationException("Filter is not boolean."));
        }
      }
    }
  };
}

#endif
