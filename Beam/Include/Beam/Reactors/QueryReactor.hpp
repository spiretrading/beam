#ifndef BEAM_QUERY_REACTOR_HPP
#define BEAM_QUERY_REACTOR_HPP
#include <aspen/Lift.hpp>
#include <aspen/Override.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/QueueReactor.hpp"

namespace Beam::Reactors {

  //! Builds a Reactor that submits a query and produces evaluates to the
  //! result.
  /*!
    \param submissionFunction The function used to submit the query.
    \param query The query to submit.
  */
  template<typename T, typename F, typename Query>
  auto QueryReactor(F&& submissionFunction, Query query) {
    auto queue = std::make_shared<Queue<T>>();
    return Aspen::override(Aspen::lift(
      [submissionFunction = std::forward<F>(submissionFunction),
          query = std::move(query), queue = std::move(queue)] {
        submissionFunction(query, queue);
        return QueueReactor(std::move(queue));
      }));
  }

  //! Builds a Query Reactor for the current value of a specified index.
  /*!
    \param submissionFunction The function used to submit the query.
    \param index The index to query.
  */
  template<typename T, typename F, typename Index>
  auto CurrentQueryReactor(F&& submissionFunction, Index index) {
    auto query = Queries::BuildCurrentQuery(std::move(index));
    return QueryReactor<T>(std::forward<F>(submissionFunction),
      std::move(query));
  }

  //! Builds a Query Reactor for real time values of a specified index.
  /*!
    \param submissionFunction The function used to submit the query.
    \param index The index to query.
  */
  template<typename T, typename F, typename Index>
  auto RealTimeQueryReactor(F&& submissionFunction, Index index) {
    auto query = Queries::BuildRealTimeQuery(std::move(index));
    return QueryReactor<T>(std::forward<F>(submissionFunction),
      std::move(query));
  }
}

#endif
