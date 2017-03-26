#ifndef BEAM_QUERYRESULT_HPP
#define BEAM_QUERYRESULT_HPP
#include <vector>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"

namespace Beam {
namespace Queries {

  /*! \struct QueryResult
      \brief Stores the result of a query.
      \tparam T The type of data returned by the query.
   */
  template<typename T>
  struct QueryResult {

    //! The type of data returned by the query.
    using Type = T;

    //! The query's unique id.
    int m_queryId;

    //! A snapshot of available data from the query.
    std::vector<Type> m_snapshot;

    //! Constructs a default QueryResult.
    QueryResult();

    //! Constructs a QueryResult.
    /*!
      \param queryId The query's unique id.
      \param snapshot The snapshot of available data from the query.
    */
    QueryResult(int queryId, std::vector<Type> snapshot);
  };

  template<typename T>
  QueryResult<T>::QueryResult()
      : m_queryId{-1} {}

  template<typename T>
  QueryResult<T>::QueryResult(int queryId, std::vector<Type> snapshot)
      : m_queryId{queryId},
        m_snapshot(std::move(snapshot)) {}
}
}

namespace Beam {
namespace Serialization {
  template<typename T>
  struct Shuttle<Queries::QueryResult<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::QueryResult<T>& value,
        unsigned int version) {
      shuttle.Shuttle("query_id", value.m_queryId);
      shuttle.Shuttle("snapshot", value.m_snapshot);
    }
  };
}
}

#endif
