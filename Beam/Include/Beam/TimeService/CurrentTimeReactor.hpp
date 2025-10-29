#ifndef BEAM_CURRENT_TIME_REACTOR_HPP
#define BEAM_CURRENT_TIME_REACTOR_HPP
#include <Aspen/Lift.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/Utilities/SharedCallable.hpp"

namespace Beam {
namespace Details {
  template<typename C>
  struct CurrentTimeCore {
    local_ptr_t<C> m_time_client;

    template<typename TF>
    CurrentTimeCore(TF&& time_client)
      : m_time_client(std::forward<TF>(time_client)) {}

    template<typename T>
    auto operator ()(T&& value) {
      return m_time_client->get_time();
    }
  };
}

  /**
   * Returns a Reactor that evaluates to the timepoint at the moment it's
   * evaluated.
   * @param time_client The TimeClient to use.
   * @param pulse The reactor used to trigger an update.
   */
  template<typename C, typename P> requires IsTimeClient<dereference_t<C>>
  auto current_time_reactor(C&& time_client, P&& pulse) {
    auto core = SharedCallable(
      std::make_unique<Details::CurrentTimeCore<std::remove_cvref_t<C>>>(
        std::forward<C>(time_client)));
    return Aspen::lift(std::move(core), std::forward<P>(pulse));
  }

  /**
   * Returns a Reactor that evaluates to the timepoint at the moment it's
   * evaluated.
   * @param time_client The TimeClient to use.
   */
  template<typename C> requires IsTimeClient<dereference_t<C>>
  auto current_time_reactor(C&& time_client) {
    return current_time_reactor(std::forward<C>(time_client), 0);
  }

  /** Returns a Reactor that evaluates to the time when it's first evaluated. */
  inline auto current_time_reactor() {
    return current_time_reactor(std::make_unique<LocalTimeClient>());
  }
}

#endif
