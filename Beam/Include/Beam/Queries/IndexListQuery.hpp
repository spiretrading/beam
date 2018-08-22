#ifndef BEAM_INDEX_LIST_QUERY_HPP
#define BEAM_INDEX_LIST_QUERY_HPP
#include <ostream>
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /** Represents a query for a list of available indexes. */
  class IndexListQuery : public SnapshotLimitedQuery, public FilteredQuery {
    protected:
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const IndexListQuery& query) {
    return out << "(" << query.GetSnapshotLimit() << " " << query.GetFilter() <<
      ")";
  }

  template<typename Shuttler>
  void IndexListQuery::Shuttle(Shuttler& shuttle, unsigned int version) {
    Beam::Serialization::Shuttle<SnapshotLimitedQuery>()(shuttle, *this,
      version);
    Beam::Serialization::Shuttle<FilteredQuery>()(shuttle, *this, version);
  }
}
}

#endif
