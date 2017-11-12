#ifndef BEAM_QUERY_REACTOR_HPP
#define BEAM_QUERY_REACTOR_HPP
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/SwitchReactor.hpp"

namespace Beam {
namespace Reactors {

  //! Builds a Reactor that submits a query and produces evaluates to the
  //! result.
  /*!
    \param submissionFunction The function used to submit the query.
    \param query The query to submit.
  */
  template<typename T, typename F, typename Query>
  auto MakeQueryReactor(F&& submissionFunction, Query query) {
    auto queue = std::make_shared<Queue<T>>();
    return MakeSwitchReactor(MakeFunctionReactor(
      [submissionFunction = std::forward<F>(submissionFunction),
          query = std::move(query), queue = std::move(queue)] {
        submissionFunction(query, queue);
        return MakeQueueReactor(std::move(queue));
      }));
  }
}
}

#endif
