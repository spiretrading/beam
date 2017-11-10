#ifndef BEAM_TIMER_REACTOR_HPP
#define BEAM_TIMER_REACTOR_HPP
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Queues/MultiQueueReader.hpp"
#include "Beam/Reactors/ConstantReactor.hpp"
#include "Beam/Reactors/FunctionReactor.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Utilities/Functional.hpp"
#include "Beam/Utilities/FunctionObject.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename TickType, typename TimerFactoryType>
  struct TimerReactorCore {
    using TimerFactory = TimerFactoryType;
    using Tick = TickType;
    using Timer = GetResultOf<TimerFactory, boost::posix_time::time_duration>;
    TimerFactory m_timerFactory;
    Timer m_timer;
    boost::posix_time::time_duration m_period;
    Tick m_ticks;
    std::shared_ptr<MultiQueueReader<Threading::Timer::Result>> m_expiryQueue;

    template<typename TimerFactoryForward>
    TimerReactorCore(TimerFactoryForward&& timerFactory)
        : m_timerFactory{std::forward<TimerFactoryForward>(timerFactory)},
          m_period{boost::posix_time::not_a_date_time},
          m_ticks{},
          m_expiryQueue{std::make_shared<
            MultiQueueReader<Threading::Timer::Result>>()} {
      m_expiryQueue->Push(Threading::Timer::Result::NONE);
    }

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
      m_timer = m_timerFactory(m_period);
      m_timer->GetPublisher().Monitor(m_expiryQueue->GetWriter());
      m_timer->Start();
    }
  };
}

  //! Makes a Reactor that increments a counter periodically.
  /*!
    \param timerFactory Builds Timers used to measure time.
    \param period The period to increment the counter.
  */
  template<typename Tick, typename TimerFactory, typename PeriodReactor>
  auto MakeTimerReactor(TimerFactory&& timerFactory, PeriodReactor&& period) {
    auto core = MakeFunctionObject(std::make_unique<
      Details::TimerReactorCore<Tick, typename std::decay<TimerFactory>::type>>(
      std::forward<TimerFactory>(timerFactory)));
    auto expiryReactor = MakeQueueReactor(
      std::static_pointer_cast<QueueReader<Threading::Timer::Result>>(
      core.GetFunction().m_expiryQueue));
    return MakeFunctionReactor(std::move(core),
      Lift(std::forward<PeriodReactor>(period)), expiryReactor);
  }

  //! Makes a Reactor that increments a counter periodically.
  /*!
    \param timerFactory Builds Timers used to measure time.
    \param period The period to increment the counter.
  */
  template<typename Tick, typename TimerFactory, typename PeriodReactor>
  auto Timer(TimerFactory&& timerFactory, PeriodReactor&& period) {
    return MakeTimerReactor<Tick>(std::forward<TimerFactory>(timerFactory),
      std::forward<PeriodReactor>(period));
  }
}
}

#endif
