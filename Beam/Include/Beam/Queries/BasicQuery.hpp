#ifndef BEAM_BASICQUERY_HPP
#define BEAM_BASICQUERY_HPP
#include <ostream>
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/InterruptableQuery.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class BasicQuery
      \brief Composes various standard query types into a query with common and
             basic functionality.
   */
  template<typename T>
  class BasicQuery : public IndexedQuery<T>, public RangedQuery,
      public SnapshotLimitedQuery, public InterruptableQuery,
      public FilteredQuery {
    public:
    protected:
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
  };

  template<typename T>
  std::ostream& operator <<(std::ostream& out, const BasicQuery<T>& query) {
    return out << "(" << query.GetIndex() << " " << query.GetRange() << " " <<
      query.GetSnapshotLimit() << " " << query.GetInterruptionPolicy() << " " <<
      query.GetFilter() << ")";
  }

  template<typename T>
  template<typename Shuttler>
  void BasicQuery<T>::Shuttle(Shuttler& shuttle, unsigned int version) {
    Beam::Serialization::Shuttle<IndexedQuery<T>>()(shuttle, *this, version);
    Beam::Serialization::Shuttle<RangedQuery>()(shuttle, *this, version);
    Beam::Serialization::Shuttle<SnapshotLimitedQuery>()(shuttle, *this,
      version);
    Beam::Serialization::Shuttle<InterruptableQuery>()(shuttle, *this, version);
    Beam::Serialization::Shuttle<FilteredQuery>()(shuttle, *this, version);
  }
}
}

#endif
