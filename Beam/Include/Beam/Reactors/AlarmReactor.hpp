#ifndef BEAM_ALARM_REACTOR_HPP
#define BEAM_ALARM_REACTOR_HPP
#include <type_traits>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
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
  template<typename TimerFactory, typename TimeClientType>
  struct AlarmReactorCore {
    using Timer = GetDereferenceType<
      GetResultOf<TimerFactory, boost::posix_time::time_duration>>;
    TimerFactory m_timerFactory;
    GetOptionalLocalPtr<TimeClientType> m_timeClient;
    std::unique_ptr<Timer> m_timer;
    boost::optional<boost::posix_time::ptime> m_expiry;
    std::shared_ptr<MultiQueueReader<Threading::Timer::Result>> m_expiryQueue;

    template<typename TimerFactoryForward, typename TimeClientForward>
    AlarmReactorCore(TimerFactoryForward&& timerFactory,
        TimeClientForward&& timeClient)
        : m_timerFactory{std::forward<TimerFactoryForward>(timerFactory)},
          m_timeClient{std::forward<TimeClientForward>(timeClient)},
          m_expiryQueue{std::make_shared<
            MultiQueueReader<Threading::Timer::Result>>()} {
      m_expiryQueue->Push(Threading::Timer::Result::NONE);
    }

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
        m_timer->GetPublisher().Monitor(m_expiryQueue->GetWriter());
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
    \param timeClient Used to get the current time.
    \param timerFactory Builds Timers used to measure time.
    \param expiry The time after which the Reactor will evaluate to
           <code>true</code>.
  */
  template<typename TimerFactory, typename TimeClient, typename ExpiryReactor>
  auto MakeAlarmReactor(TimeClient&& timeClient, TimerFactory&& timerFactory,
      ExpiryReactor&& expiry) {
    using BaseTimerFactory = typename std::decay<TimerFactory>::type;
    using BaseTimeClient = typename std::decay<TimeClient>::type;
    auto core = MakeFunctionObject(std::make_unique<
      AlarmReactorCore<BaseTimerFactory, BaseTimeClient>>(
      std::forward<TimerFactory>(timerFactory),
      std::forward<TimeClient>(timeClient)));
    auto timerReactor = MakeQueueReactor(
      std::static_pointer_cast<QueueReader<Threading::Timer::Result>>(
      core.GetFunction().m_expiryQueue));
    return MakeFunctionReactor(std::move(core),
      Lift(std::forward<ExpiryReactor>(expiry)), std::move(timerReactor));
  }

  //! Makes a Reactor that evaluates to <code>true</code> after a specified
  //! time.
  /*!
    \param timeClient Used to get the current time.
    \param timerFactory Builds Timers used to measure time.
    \param expiry The time after which the Reactor will evaluate to
           <code>true</code>.
  */
  template<typename TimerFactory, typename TimeClient, typename ExpiryReactor>
  auto Alarm(TimeClient&& timeClient, TimerFactory&& timerFactory,
      ExpiryReactor&& expiry) {
    return MakeAlarmReactor(std::forward<TimeClient>(timeClient),
      std::forward<TimerFactory>(timerFactory),
      std::forward<ExpiryReactor>(expiry));
  }
}
}

#endif
