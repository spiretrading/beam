#ifndef BEAM_TIMERREACTOR_HPP
#define BEAM_TIMERREACTOR_HPP
#include <type_traits>
#include <utility>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Queues/CallbackQueue.hpp"
#include "Beam/Queues/StatePublisher.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/PublisherReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Utilities/Functional.hpp"
#include "Beam/Utilities/FunctionObject.hpp"

namespace Beam {
namespace Reactors {
  template<typename TimerFactoryType, typename TickType>
  struct TimerReactorCore {
    using TimerFactory = TimerFactoryType;
    using Tick = TickType;
    using Timer = GetDereferenceType<
      GetResultOf<TimerFactory, boost::posix_time::time_duration>>;
    TimerFactory m_timerFactory;
    std::unique_ptr<Timer> m_timer;
    boost::optional<boost::posix_time::time_duration> m_period;
    StatePublisher<Threading::Timer::Result> m_expiryPublisher;
    std::shared_ptr<Reactor<Threading::Timer::Result>> m_expiryReactor;
    Tick m_ticks;
    CallbackQueue m_callbacks;

    template<typename TimerFactoryForward>
    TimerReactorCore(TimerFactoryForward&& timerFactory)
        : m_timerFactory(std::forward<TimerFactoryForward>(timerFactory)),
          m_expiryPublisher(Threading::Timer::Result::NONE),
          m_expiryReactor(MakePublisherReactor(&m_expiryPublisher)),
          m_ticks(Tick()) {}

    Tick operator ()(const boost::posix_time::time_duration& period,
        Threading::Timer::Result timerResult) {
      if(period != m_period) {
        if(m_timer != nullptr) {
          m_timer->Cancel();
        }
        m_period = period;
        ResetTimer();
      } else if(timerResult == Threading::Timer::Result::EXPIRED) {
        ++m_ticks;
        ResetTimer();
      }
      return m_ticks;
    }

    void ResetTimer() {
      m_timer = m_timerFactory(*m_period);
      m_timer->GetPublisher().Monitor(
        m_callbacks.GetSlot<Threading::Timer::Result>(
        [=] (Threading::Timer::Result result) {
          m_expiryPublisher.Push(result);
        }));
      m_timer->Start();
    }
  };

  //! Builds a Reactor that repeatedly increments a value after a specified
  //! time period.
  /*!
    \param timerFactory Used to build the Timer used to keep time.
    \param period The period after which the value is incremented.
  */
  template<typename Tick, typename TimerFactory, typename PeriodReactor>
  std::tuple<std::shared_ptr<Reactor<Tick>>, std::shared_ptr<Event>>
      MakeTimerReactor(TimerFactory&& timerFactory, PeriodReactor&& period) {
    using BaseTimerFactory = typename std::decay<TimerFactory>::type;
    auto core = MakeFunctionObject(std::make_unique<
      TimerReactorCore<BaseTimerFactory, Tick>>(
      std::forward<TimerFactory>(timerFactory)));
    auto expiryReactor = core.GetFunction().m_expiryReactor;
    auto expiryEvent = std::dynamic_pointer_cast<Event>(
      core.GetFunction().m_expiryReactor);
    auto reactor = MakeFunctionReactor(std::move(core),
      std::forward<PeriodReactor>(period), std::move(expiryReactor));
    return std::make_tuple(reactor, expiryEvent);
  }
}
}

#endif
