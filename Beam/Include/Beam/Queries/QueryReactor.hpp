#ifndef BEAM_QUERY_REACTOR_HPP
#define BEAM_QUERY_REACTOR_HPP
#include <Aspen/Lift.hpp>
#include <Aspen/Override.hpp>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReactor.hpp"

namespace Beam {

  /**
   * Returns a Reactor that submits a query and evaluates to the result.
   * @param submission The function used to submit the query.
   * @param query The query to submit.
   */
  template<typename T, typename F, typename Query>
  auto query_reactor(F&& submission, Query&& query) {
    return Aspen::override(Aspen::lift(
      [submission = std::forward<F>(submission)] (
          const Aspen::reactor_result_t<Query>& query) {
        auto queue = std::make_shared<Queue<T>>();
        submission(query, queue);
        return Aspen::Shared(QueueReactor(std::move(queue)));
      }, std::forward<Query>(query)));
  }

  /**
   * Returns a Query Reactor for the current value of a specified index.
   * @param submission The function used to submit the query.
   * @param index The index to query.
   */
  template<typename T, typename F, typename Index>
  auto current_query_reactor(F&& submission, Index index) {
    auto query = make_current_query(std::move(index));
    return query_reactor<T>(std::forward<F>(submission), std::move(query));
  }

  /**
   * Returns a Query Reactor for real time values of a specified index.
   * @param submission The function used to submit the query.
   * @param index The index to query.
   */
  template<typename T, typename F, typename Index>
  auto real_time_query_reactor(F&& submission, Index index) {
    auto query = make_real_time_query(std::move(index));
    return query_reactor<T>(std::forward<F>(submission), std::move(query));
  }
}

#endif
