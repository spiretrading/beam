#ifndef BEAM_CURRENT_TIME_REACTOR_HPP
#define BEAM_CURRENT_TIME_REACTOR_HPP
#include <Aspen/Lift.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/TimeClient.hpp"
#include "Beam/Utilities/FunctionObject.hpp"

namespace Beam::Reactors {
namespace Details {
  template<typename TimeClientType>
  struct CurrentTimeCore {
    GetOptionalLocalPtr<TimeClientType> m_timeClient;

    template<typename TimeClientForward>
    CurrentTimeCore(TimeClientForward&& timeClient)
        : m_timeClient{std::forward<TimeClientForward>(timeClient)} {
      m_timeClient->Open();
    }

    template<typename T>
    auto operator ()(T&& value) {
      return m_timeClient->GetTime();
    }
  };
}

  /**
   * Returns a Reactor that evaluates to the timepoint at the moment it's
   * evaluated.
   * @param timeClient The TimeClient to use.
   * @param pulse The reactor used to trigger an update.
   */
  template<typename TimeClient, typename P>
  auto CurrentTimeReactor(TimeClient&& timeClient, P&& pulse) {
    auto core = MakeFunctionObject(std::make_unique<
      Details::CurrentTimeCore<std::decay_t<TimeClient>>>(
      std::forward<TimeClient>(timeClient)));
    return Aspen::lift(std::move(core), std::forward<P>(pulse));
  }

  /**
   * Returns a Reactor that evaluates to the timepoint at the moment it's
   * evaluated.
   * @param timeClient The TimeClient to use.
   */
  template<typename TimeClient>
  auto CurrentTimeReactor(TimeClient&& timeClient) {
    return CurrentTimeReactor(std::forward<TimeClient>(timeClient),
      Aspen::constant(0));
  }

  //! Returns a Reactor that evaluates to the time when it's first evaluated.
  inline auto CurrentTimeReactor() {
    return CurrentTimeReactor(std::make_unique<TimeService::LocalTimeClient>());
  }
}

#endif
