#ifndef BEAM_TIMER_REACTOR_HPP
#define BEAM_TIMER_REACTOR_HPP
#include <memory>
#include <type_traits>
#include <Aspen/Chain.hpp>
#include <Aspen/Lift.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Utilities/FunctionObject.hpp"

namespace Beam::Reactors {
namespace Details {
  template<typename TickType, typename TimerFactoryType>
  struct TimerReactorCore {
    using TimerFactory = TimerFactoryType;
    using Tick = TickType;
    using Timer = std::invoke_result_t<TimerFactory,
      boost::posix_time::time_duration>;
    TimerFactory m_timerFactory;
    Timer m_timer;
    boost::posix_time::time_duration m_period;
    Tick m_ticks;
    std::shared_ptr<MultiQueueWriter<Threading::Timer::Result>> m_expiryQueue;

    template<typename TimerFactoryForward>
    TimerReactorCore(TimerFactoryForward&& timerFactory)
      : m_timerFactory(std::forward<TimerFactoryForward>(timerFactory)),
        m_period(boost::posix_time::not_a_date_time),
        m_ticks(),
        m_expiryQueue(std::make_shared<
          MultiQueueWriter<Threading::Timer::Result>>()) {}

    std::optional<Tick> operator ()(boost::posix_time::time_duration period,
        Threading::Timer::Result timerResult) {
      if(period != m_period) {
        auto hasTimer = m_timer != nullptr;
        if(hasTimer) {
          m_timer->Cancel();
        }
        m_period = period;
        ResetTimer();
        if(!hasTimer) {
          return m_ticks;
        }
      } else if(timerResult == Threading::Timer::Result::EXPIRED) {
        ++m_ticks;
        ResetTimer();
        return m_ticks;
      }
      return std::nullopt;
    }

    void ResetTimer() {
      m_timer = m_timerFactory(m_period);
      m_timer->GetPublisher().Monitor(m_expiryQueue->GetWriter());
      m_timer->Start();
    }
  };
}

  /**
   * Makes a Reactor that increments a counter periodically.
   * @param timerFactory Builds Timers used to measure time.
   * @param period The period to increment the counter.
   */
  template<typename Tick, typename TimerFactory, typename PeriodReactor>
  auto TimerReactor(TimerFactory&& timerFactory, PeriodReactor&& period) {
    auto core = MakeFunctionObject(std::make_unique<
      Details::TimerReactorCore<Tick, std::decay_t<TimerFactory>>>(
      std::forward<TimerFactory>(timerFactory)));
    return Aspen::lift(std::move(core), std::forward<PeriodReactor>(period),
      Aspen::Chain(Threading::Timer::Result(Threading::Timer::Result::NONE),
      QueueReactor(core.GetFunction().m_expiryQueue)));
  }
}

#endif
