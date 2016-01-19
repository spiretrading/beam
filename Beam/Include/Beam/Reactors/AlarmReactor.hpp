#ifndef BEAM_ALARMREACTOR_HPP
#define BEAM_ALARMREACTOR_HPP
#include <type_traits>
#include <utility>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
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
  template<typename TimerFactory, typename TimeClientType>
  struct AlarmReactorCore {
    using Timer = GetDereferenceType<
      GetResultOf<TimerFactory, boost::posix_time::time_duration>>;
    TimerFactory m_timerFactory;
    GetOptionalLocalPtr<TimeClientType> m_timeClient;
    std::unique_ptr<Timer> m_timer;
    boost::optional<boost::posix_time::ptime> m_expiry;
    StatePublisher<Threading::Timer::Result> m_expiryPublisher;
    std::shared_ptr<Reactor<Threading::Timer::Result>> m_expiryReactor;
    CallbackQueue m_callbacks;

    template<typename TimerFactoryForward, typename TimeClientForward>
    AlarmReactorCore(TimerFactoryForward&& timerFactory,
        TimeClientForward&& timeClient)
        : m_timerFactory(std::forward<TimerFactoryForward>(timerFactory)),
          m_timeClient(std::forward<TimeClientForward>(timeClient)),
          m_expiryPublisher(Threading::Timer::Result::NONE),
          m_expiryReactor(MakePublisherReactor(&m_expiryPublisher)) {}

    bool operator ()(const boost::posix_time::ptime& expiry,
        Threading::Timer::Result timerResult) {
      if(expiry != m_expiry) {
        if(m_timer != nullptr) {
          m_timer->Cancel();
          m_timer.reset();
        }
        auto currentTime = m_timeClient->GetTime();
        if(expiry <= currentTime) {
          return true;
        }
        m_expiry = expiry;
        m_timer = m_timerFactory(expiry - currentTime);
        m_timer->GetPublisher().Monitor(
          m_callbacks.GetSlot<Threading::Timer::Result>(
          [=] (Threading::Timer::Result result) {
            m_expiryPublisher.Push(result);
          }));
        m_timer->Start();
        return false;
      } else if(timerResult == Threading::Timer::Result::EXPIRED) {
        m_expiry = boost::none;
        m_timer.reset();
        return true;
      }
      return false;
    }
  };

  //! Makes a Reactor that evaluates to <code>true</code> after a specified
  //! time.
  /*!
    \param timerFactory Builds Timers used to measure time.
    \param timeClient Used to get the current time.
    \param expiry The time after which the Reactor will evaluate to
           <code>true</code>.
  */
  template<typename TimerFactory, typename TimeClient, typename ExpiryReactor>
  std::tuple<std::shared_ptr<Reactor<bool>>, std::shared_ptr<Event>>
      MakeAlarmReactor(TimerFactory&& timerFactory, TimeClient&& timeClient,
      ExpiryReactor&& expiry) {
    using BaseTimerFactory = typename std::decay<TimerFactory>::type;
    using BaseTimeClient = typename std::decay<TimeClient>::type;
    auto core = MakeFunctionObject(std::make_unique<
      AlarmReactorCore<BaseTimerFactory, BaseTimeClient>>(
      std::forward<TimerFactory>(timerFactory),
      std::forward<TimeClient>(timeClient)));
    auto timerReactor = core.GetFunction().m_expiryReactor;
    auto alarmEvent = std::dynamic_pointer_cast<Event>(
      core.GetFunction().m_expiryReactor);
    auto reactor = MakeFunctionReactor(std::move(core),
      std::forward<ExpiryReactor>(expiry), std::move(timerReactor));
    return std::make_tuple(reactor, alarmEvent);
  }
}
}

#endif
