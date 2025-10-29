#ifndef BEAM_TIMER_REACTOR_HPP
#define BEAM_TIMER_REACTOR_HPP
#include <memory>
#include <type_traits>
#include <Aspen/Chain.hpp>
#include <Aspen/Lift.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/QueueReactor.hpp"
#include "Beam/TimeService/Timer.hpp"
#include "Beam/Utilities/SharedCallable.hpp"

namespace Beam {
namespace Details {
  template<typename TickType, typename TimerFactoryType>
  struct TimerReactorCore {
    using TimerFactory = TimerFactoryType;
    using Tick = TickType;
    using Timer =
      std::invoke_result_t<TimerFactory, boost::posix_time::time_duration>;
    TimerFactory m_timer_factory;
    Timer m_timer;
    boost::posix_time::time_duration m_period;
    Tick m_ticks;
    std::shared_ptr<MultiQueueWriter<Beam::Timer::Result>> m_expiry_queue;

    template<typename TF>
    TimerReactorCore(TF&& timer_factory)
      : m_timer_factory(std::forward<TF>(timer_factory)),
        m_period(boost::posix_time::not_a_date_time),
        m_ticks(),
        m_expiry_queue(
          std::make_shared<MultiQueueWriter<Beam::Timer::Result>>()) {}

    std::optional<Tick> operator ()(
        boost::posix_time::time_duration period, Beam::Timer::Result result) {
      if(period != m_period) {
        auto has_timer = m_timer != nullptr;
        if(has_timer) {
          m_timer->cancel();
        }
        m_period = period;
        reset();
        if(!has_timer) {
          return m_ticks;
        }
      } else if(result == Beam::Timer::Result::EXPIRED) {
        ++m_ticks;
        reset();
        return m_ticks;
      }
      return std::nullopt;
    }

    void reset() {
      m_timer = m_timer_factory(m_period);
      m_timer->get_publisher().monitor(m_expiry_queue->get_writer());
      m_timer->start();
    }
  };
}

  /**
   * Makes a Reactor that increments a counter periodically.
   * @param timer_factory Constructs Timers used to measure time.
   * @param period The period to increment the counter.
   */
  template<typename T, typename F, typename R>
  auto timer_reactor(F&& timer_factory, R&& period) {
    auto core = SharedCallable(
      std::make_unique<Details::TimerReactorCore<T, std::remove_cvref_t<F>>>(
        std::forward<F>(timer_factory)));
    auto& expiry_queue = core.get_callable().m_expiry_queue;
    return Aspen::lift(std::move(core), std::forward<R>(period),
      Aspen::Chain(Timer::Result(Timer::Result::NONE),
        QueueReactor(expiry_queue)));
  }
}

#endif
