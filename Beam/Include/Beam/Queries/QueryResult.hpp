#ifndef BEAM_QUERY_RESULT_HPP
#define BEAM_QUERY_RESULT_HPP
#include <vector>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"

namespace Beam {

  /**
   * Stores the result of a query.
   * @tparam T The type of data returned by the query.
   */
  template<typename T>
  struct QueryResult {

    /** The type of data returned by the query. */
    using Type = T;

    /** The query's unique id. */
    int m_id;

    /** A snapshot of available data from the query. */
    std::vector<Type> m_snapshot;

    /** Constructs a default QueryResult. */
    QueryResult() noexcept;

    /**
     * Constructs a QueryResult.
     * @param id The query's unique id.
     * @param snapshot The snapshot of available data from the query.
     */
    QueryResult(int id, std::vector<Type> snapshot) noexcept;

    bool operator ==(const QueryResult&) const = default;
  };

  template<typename T>
  QueryResult<T>::QueryResult() noexcept
    : m_id(-1) {}

  template<typename T>
  QueryResult<T>::QueryResult(int id, std::vector<Type> snapshot) noexcept
    : m_id(id),
      m_snapshot(std::move(snapshot)) {}

  template<typename T>
  struct Shuttle<QueryResult<T>> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, QueryResult<T>& value, unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("snapshot", value.m_snapshot);
    }
  };
}

#endif
