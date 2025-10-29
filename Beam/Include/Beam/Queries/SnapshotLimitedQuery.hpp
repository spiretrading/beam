#ifndef BEAM_SNAPSHOT_LIMITED_QUERY_HPP
#define BEAM_SNAPSHOT_LIMITED_QUERY_HPP
#include <ostream>
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Queries for a snapshot with a specified limit. */
  class SnapshotLimitedQuery {
    public:

      /** Constructs a SnapshotLimitedQuery specifying an empty snapshot. */
      SnapshotLimitedQuery() = default;

      /**
       * Constructs a SnapshotLimitedQuery with a specified SnapshotLimit.
       * @param limit The SnapshotLimit.
       */
      explicit SnapshotLimitedQuery(SnapshotLimit limit) noexcept;

      /** Returns the SnapshotLimit. */
      const SnapshotLimit& get_snapshot_limit() const;

      /** Sets the SnapshotLimit. */
      void set_snapshot_limit(const SnapshotLimit& limit);

      /**
       * Sets the SnapshotLimit.
       * @param type The Type of limit.
       * @param size The size of the limit.
       */
      void set_snapshot_limit(SnapshotLimit::Type type, int size);

      bool operator ==(const SnapshotLimitedQuery&) const = default;

    private:
      friend struct Shuttle<SnapshotLimitedQuery>;
      SnapshotLimit m_snapshot_limit;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const SnapshotLimitedQuery& query) {
    return out << query.get_snapshot_limit();
  }

  inline SnapshotLimitedQuery::SnapshotLimitedQuery(
    SnapshotLimit limit) noexcept
    : m_snapshot_limit(limit) {}

  inline const SnapshotLimit& SnapshotLimitedQuery::get_snapshot_limit() const {
    return m_snapshot_limit;
  }

  inline void SnapshotLimitedQuery::set_snapshot_limit(
      const SnapshotLimit& limit) {
    m_snapshot_limit = limit;
  }

  inline void SnapshotLimitedQuery::set_snapshot_limit(
      SnapshotLimit::Type type, int size) {
    set_snapshot_limit(SnapshotLimit(type, size));
  }

  template<>
  struct Shuttle<SnapshotLimitedQuery> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, SnapshotLimitedQuery& value, unsigned int version) const {
      shuttle.shuttle("snapshot_limit", value.m_snapshot_limit);
    }
  };
}

#endif
