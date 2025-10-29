#ifndef BEAM_RANGED_QUERY_HPP
#define BEAM_RANGED_QUERY_HPP
#include <ostream>
#include <utility>
#include "Beam/Queries/Range.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Queries for data over a specified Range. */
  class RangedQuery {
    public:

      /** Constructs a RangedQuery over an empty Range. */
      RangedQuery() = default;

      /**
       * Constructs a RangedQuery over a specified Range.
       * @param range The Range to Query over.
       */
      explicit RangedQuery(Range range) noexcept;

      /** Returns the Range to Query. */
      const Range& get_range() const;

      /** Sets the Range to Query. */
      void set_range(const Range& range);

      /**
       * Sets the Range to Query.
       * @param start The start point.
       * @param end The end point.
       */
      void set_range(const Range::Point& start, const Range::Point& end);

      bool operator ==(const RangedQuery&) const = default;

    private:
      friend struct Shuttle<RangedQuery>;
      Range m_range;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const RangedQuery& query) {
    return out << query.get_range();
  }

  inline RangedQuery::RangedQuery(Range range) noexcept
    : m_range(std::move(range)) {}

  inline const Range& RangedQuery::get_range() const {
    return m_range;
  }

  inline void RangedQuery::set_range(const Range& range) {
    m_range = range;
  }

  inline void RangedQuery::set_range(
      const Range::Point& start, const Range::Point& end) {
    set_range(Range(start, end));
  }

  template<>
  struct Shuttle<RangedQuery> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, RangedQuery& value, unsigned int version) const {
      shuttle.shuttle("range", value.m_range);
    }
  };
}

#endif
