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
#include "Beam/Reactors/QueueReactor.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Utilities/Functional.hpp"
#include "Beam/Utilities/FunctionObject.hpp"

namespace Beam::Reactors {
namespace Details {
  template<typename TimerFactory, typename TimeClientType>
  struct AlarmReactorCore {
    using Timer = std::invoke_result_t<
      TimerFactory, boost::posix_time::time_duration>;
    TimerFactory m_timerFactory;
    GetOptionalLocalPtr<TimeClientType> m_timeClient;
    std::optional<GetOptionalLocalPtr<Timer>> m_timer;
    boost::posix_time::ptime m_expiry;
    std::shared_ptr<MultiQueueWriter<Threading::Timer::Result>> m_expiryQueue;

    template<typename TimerFactoryForward, typename TimeClientForward>
    AlarmReactorCore(TimerFactoryForward&& timerFactory,
      TimeClientForward&& timeClient)
      : m_timerFactory(std::forward<TimerFactoryForward>(timerFactory)),
        m_timeClient(std::forward<TimeClientForward>(timeClient)),
        m_expiry(boost::posix_time::not_a_date_time),
        m_expiryQueue(std::make_shared<
          MultiQueueWriter<Threading::Timer::Result>>()) {}

    bool operator ()(const boost::posix_time::ptime& expiry,
        Aspen::State expiryState, Threading::Timer::Result timerResult) {
      if(expiry != m_expiry) {
        auto hasTimer = m_timer.has_value();
        if(hasTimer) {
          (*m_timer)->Cancel();
          m_timer = std::nullopt;
        }
        auto currentTime = m_timeClient->GetTime();
        if(expiry <= currentTime) {
          if(Aspen::is_complete(expiryState)) {
            m_expiryQueue->Break();
          }
          return true;
        }
        m_expiry = expiry;
        m_timer.emplace(m_timerFactory(expiry - currentTime));
        (*m_timer)->GetPublisher().Monitor(m_expiryQueue->GetWriter());
        (*m_timer)->Start();
        return false;
      } else if(timerResult == Threading::Timer::Result::EXPIRED) {
        m_expiry = boost::posix_time::not_a_date_time;
        m_timer = std::nullopt;
        if(Aspen::is_complete(expiryState)) {
          m_expiryQueue->Break();
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
   * @param timeClient Used to get the current time.
   * @param timerFactory Builds Timers used to measure time.
   * @param expiry The time after which the Reactor will evaluate to
   *        <code>true</code>.
   */
  template<typename TimerFactory, typename TimeClient, typename ExpiryReactor>
  auto AlarmReactor(TimeClient&& timeClient, TimerFactory&& timerFactory,
      ExpiryReactor&& expiry) {
    auto expiryReactor = Aspen::Shared(std::forward<ExpiryReactor>(expiry));
    auto expiryState = Aspen::StateReactor(expiryReactor);
    auto core = MakeFunctionObject(std::make_unique<Details::AlarmReactorCore<
      std::decay_t<TimerFactory>, std::decay_t<TimeClient>>>(
      std::forward<TimerFactory>(timerFactory),
      std::forward<TimeClient>(timeClient)));
    return Aspen::lift(std::move(core), expiryReactor, expiryState,
      Aspen::Chain(Threading::Timer::Result(Threading::Timer::Result::NONE),
      QueueReactor(core.GetFunction().m_expiryQueue)));
  }
}

#endif
