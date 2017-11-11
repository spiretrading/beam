#ifndef BEAM_CURRENT_TIME_REACTOR_HPP
#define BEAM_CURRENT_TIME_REACTOR_HPP
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/TimeService/LocalTimeClient.hpp"
#include "Beam/TimeService/TimeClient.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename TimeClientType>
  struct CurrentTimeCore {
    GetOptionalLocalPtr<TimeClientType> m_timeClient;

    template<typename TimeClientForward>
    CurrentTimeCore(TimeClientForward&& timeClient)
        : m_timeClient{std::forward<TimeClientForward>(timeClient)} {}

    boost::posix_time::ptime operator ()() {
      m_timeClient->Open();
      return m_timeClient->GetTime();
    }
  };
}

  //! Returns a Reactor that evaluates to the time when it's first evaluated.
  /*!
    \param timeClient The TimeClient to use.
  */
  template<typename TimeClient>
  auto MakeCurrentTimeReactor(TimeClient&& timeClient) {
    auto core = MakeFunctionObject(std::make_unique<
      Details::CurrentTimeCore<typename std::decay<TimeClient>::type>>(
      std::forward<TimeClient>(timeClient)));
    return MakeFunctionReactor(std::move(core));
  }

  //! Returns a Reactor that evaluates to the time when it's first evaluated.
  /*!
    \param timeClient The TimeClient to use.
  */
  template<typename TimeClient>
  auto CurrentTime(TimeClient&& timeClient) {
    return MakeCurrentTimeReactor(std::forward<TimeClient>(timeClient));
  }

  //! Returns a Reactor that evaluates to the time when it's first evaluated.
  inline auto MakeCurrentTimeReactor() {
    return MakeCurrentTimeReactor(
      std::make_unique<TimeService::LocalTimeClient>());
  }

  //! Returns a Reactor that evaluates to the time when it's first evaluated.
  inline auto CurrentTime() {
    return MakeCurrentTimeReactor();
  }
}
}

#endif
