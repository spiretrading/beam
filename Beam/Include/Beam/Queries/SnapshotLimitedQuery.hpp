#ifndef BEAM_SNAPSHOTLIMITEDQUERY_HPP
#define BEAM_SNAPSHOTLIMITEDQUERY_HPP
#include <ostream>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class SnapshotLimitedQuery
      \brief Queries for a snapshot with a specified limit.
   */
  class SnapshotLimitedQuery {
    public:

      //! Constructs a SnapshotLimitedQuery specifying an empty snapshot.
      SnapshotLimitedQuery() = default;

      //! Constructs a SnapshotLimitedQuery with a specified SnapshotLimit.
      /*!
        \param limit The SnapshotLimit.
      */
      SnapshotLimitedQuery(const SnapshotLimit& limit);

      //! Returns the SnapshotLimit.
      const SnapshotLimit& GetSnapshotLimit() const;

      //! Sets the SnapshotLimit.
      void SetSnapshotLimit(const SnapshotLimit& limit);

      //! Sets the SnapshotLimit.
      /*!
        \param type The Type of limit.
        \param size The size of the limit.
      */
      void SetSnapshotLimit(SnapshotLimit::Type type, int size);

    private:
      friend struct Serialization::Shuttle<SnapshotLimitedQuery>;
      SnapshotLimit m_snapshotLimit;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const SnapshotLimitedQuery& query) {
    return out << query.GetSnapshotLimit();
  }

  inline SnapshotLimitedQuery::SnapshotLimitedQuery(const SnapshotLimit& limit)
      : m_snapshotLimit{limit} {}

  inline const SnapshotLimit& SnapshotLimitedQuery::GetSnapshotLimit() const {
    return m_snapshotLimit;
  }

  inline void SnapshotLimitedQuery::SetSnapshotLimit(
      const SnapshotLimit& limit) {
    m_snapshotLimit = limit;
  }

  inline void SnapshotLimitedQuery::SetSnapshotLimit(SnapshotLimit::Type type,
      int size) {
    SetSnapshotLimit(SnapshotLimit(type, size));
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Queries::SnapshotLimitedQuery> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::SnapshotLimitedQuery& value,
        unsigned int version) {
      shuttle.Shuttle("snapshot_limit", value.m_snapshotLimit);
    }
  };
}
}

#endif
