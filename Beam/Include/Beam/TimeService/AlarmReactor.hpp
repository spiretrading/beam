#ifndef BEAM_ALARM_REACTOR_HPP
#define BEAM_ALARM_REACTOR_HPP
#include <memory>
#include <type_traits>
#include <Aspen/Chain.hpp>
#include <Aspen/Lift.hpp>
#include <Aspen/Shared.hpp>
#include <Aspen/StateReactor.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/QueueReactor.hpp"
#include "Beam/TimeService/TimeClient.hpp"
#include "Beam/TimeService/Timer.hpp"
#include "Beam/Utilities/SharedCallable.hpp"

namespace Beam {
namespace Details {
  template<typename TimerFactory, typename TimeClientType>
  struct AlarmReactorCore {
    using Timer =
      std::invoke_result_t<TimerFactory, boost::posix_time::time_duration>;
    TimerFactory m_timer_factory;
    local_ptr_t<TimeClientType> m_time_client;
    std::optional<local_ptr_t<Timer>> m_timer;
    boost::posix_time::ptime m_expiry;
    std::shared_ptr<MultiQueueWriter<Beam::Timer::Result>> m_expiry_queue;

    template<typename TF, typename CF>
    AlarmReactorCore(TF&& timer_factory, CF&& time_client)
      : m_timer_factory(std::forward<TF>(timer_factory)),
        m_time_client(std::forward<CF>(time_client)),
        m_expiry(boost::posix_time::not_a_date_time),
        m_expiry_queue(
          std::make_shared<MultiQueueWriter<Beam::Timer::Result>>()) {}

    bool operator ()(boost::posix_time::ptime expiry, Aspen::State expiry_state,
        Beam::Timer::Result timer_result) {
      if(expiry != m_expiry) {
        auto has_timer = m_timer.has_value();
        if(has_timer) {
          (*m_timer)->cancel();
          m_timer = std::nullopt;
        }
        auto current_time = m_time_client->get_time();
        if(expiry <= current_time) {
          if(Aspen::is_complete(expiry_state)) {
            m_expiry_queue->close();
          }
          return true;
        }
        m_expiry = expiry;
        m_timer.emplace(m_timer_factory(expiry - current_time));
        (*m_timer)->get_publisher().monitor(m_expiry_queue->get_writer());
        (*m_timer)->start();
        return false;
      } else if(timer_result == Beam::Timer::Result::EXPIRED) {
        m_expiry = boost::posix_time::not_a_date_time;
        m_timer = std::nullopt;
        if(Aspen::is_complete(expiry_state)) {
          m_expiry_queue->close();
        }
        return true;
      }
      return false;
    }
  };
}

  /**
   * Makes a Reactor that evaluates to <code>true</code> after a specified
   * time.
   * @param time_client Used to get the current time.
   * @param timer_factory Constructs Timers used to measure time.
   * @param expiry The time after which the Reactor will evaluate to
   *        <code>true</code>.
   */
  template<typename F, typename C, typename R> requires
    std::is_invocable_v<F, boost::posix_time::time_duration> &&
      IsTimer<dereference_t<
        std::invoke_result_t<F, boost::posix_time::time_duration>>> &&
          IsTimeClient<dereference_t<C>>
  auto alarm_reactor(C&& time_client, F&& timer_factory, R&& expiry) {
    auto expiry_reactor = Aspen::Shared(std::forward<R>(expiry));
    auto expiry_state = Aspen::StateReactor(expiry_reactor);
    auto core = SharedCallable(std::make_unique<Details::AlarmReactorCore<
      std::remove_cvref_t<F>, std::remove_cvref_t<C>>>(
        std::forward<F>(timer_factory), std::forward<C>(time_client)));
    auto& expiry_queue = core.get_callable().m_expiry_queue;
    return Aspen::lift(std::move(core), expiry_reactor, expiry_state,
      Aspen::Chain(Beam::Timer::Result(Beam::Timer::Result::NONE),
        QueueReactor(expiry_queue)));
  }
}

#endif
