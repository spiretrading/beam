#ifndef BEAM_TIMED_CONDITION_VARIABLE_HPP
#define BEAM_TIMED_CONDITION_VARIABLE_HPP
#include <atomic>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Threading/TimeoutException.hpp"

namespace Beam::Threading {

  /** Implements a condition variable that can wait for a specified time period.
   */
  class TimedConditionVariable : private boost::noncopyable {
    public:

      //! Constructs a TimedConditionVariable.
      /*!
        \param timerThreadPool The TimerThreadPool used to perform timed waits.
      */
      TimedConditionVariable(Ref<TimerThreadPool> timerThreadPool);

      //! Suspends the current Routine until a notification is received.
      /*!
        \param lock The lock synchronizing the notification event.
      */
      template<typename... Lock>
      void wait(Lock&... lock);

      //! Suspends the current Routine for a specified amount of time.
      /*!
        \param duration The amount of time to wait.
        \param lock The lock synchronizing the notification event.
      */
      template<typename... Lock>
      void timed_wait(const boost::posix_time::time_duration& duration,
        Lock&... lock);

      //! Triggers a notification event for a single suspended Routine.
      void notify_one();

      //! Triggers a notification event for all suspended Routine.
      void notify_all();

    private:
      struct WaitEntry {
        std::atomic_bool m_isWaiting;
        std::atomic_bool m_isTimerStarted;
        LiveTimer m_timer;
        Routines::Async<void> m_timerResult;
        ConditionVariable m_condition;

        WaitEntry(Ref<TimerThreadPool> timerThreadPool);
        WaitEntry(const boost::posix_time::time_duration& duration,
          Ref<TimerThreadPool> timerThreadPool);
      };
      TimerThreadPool* m_timerThreadPool;
      Sync<std::deque<WaitEntry*>> m_waitEntries;

      void NotifyWaitEntry(WaitEntry& waitEntry);
  };

  inline TimedConditionVariable::WaitEntry::WaitEntry(
      Ref<TimerThreadPool> timerThreadPool)
      : m_isWaiting(true),
        m_isTimerStarted(false),
        m_timer(boost::posix_time::seconds(0), Ref(timerThreadPool)) {}

  inline TimedConditionVariable::WaitEntry::WaitEntry(
      const boost::posix_time::time_duration& duration,
      Ref<TimerThreadPool> timerThreadPool)
      : m_isWaiting(true),
        m_isTimerStarted(false),
        m_timer(duration, Ref(timerThreadPool)) {}

  inline TimedConditionVariable::TimedConditionVariable(
      Ref<TimerThreadPool> timerThreadPool)
      : m_timerThreadPool(timerThreadPool.Get()) {}

  template<typename... Lock>
  void TimedConditionVariable::wait(Lock&... lock) {
    auto waitEntry = WaitEntry(Ref(*m_timerThreadPool));
    m_waitEntries.With(
      [&] (auto& waitEntries) {
        waitEntries.push_back(&waitEntry);
      });
    waitEntry.m_condition.wait(lock...);
  }
  template<typename... Lock>
  void TimedConditionVariable::timed_wait(
      const boost::posix_time::time_duration& duration, Lock&... lock) {
    auto waitEntry = WaitEntry(duration, Ref(*m_timerThreadPool));
    m_waitEntries.With(
      [&] (auto& waitEntries) {
        waitEntries.push_back(&waitEntry);
      });
    auto waitRoutine = Routines::Spawn(
      [&] {
        waitEntry.m_timer.Start();
        auto isTimerStarted = waitEntry.m_isTimerStarted.exchange(true);
        if(isTimerStarted) {
          waitEntry.m_timer.Cancel();
          return;
        }
        waitEntry.m_timer.Wait();
        auto isWaiting = waitEntry.m_isWaiting.exchange(false);
        if(isWaiting) {
          m_waitEntries.With(
            [&] (auto& waitEntries) {
              auto entryIterator = std::find(waitEntries.begin(),
                waitEntries.end(), &waitEntry);
              if(entryIterator != waitEntries.end()) {
                waitEntries.erase(entryIterator);
              }
            });
          waitEntry.m_timerResult.GetEval().SetException(TimeoutException());
          waitEntry.m_condition.notify_one();
        }
      });
    waitEntry.m_condition.wait(lock...);
    Routines::Wait(waitRoutine);
    waitEntry.m_timerResult.Get();
  }

  inline void TimedConditionVariable::notify_one() {
    auto waitEntry = static_cast<WaitEntry*>(nullptr);
    m_waitEntries.With(
      [&] (auto& waitEntries) {
        if(waitEntries.empty()) {
          waitEntry = nullptr;
        } else {
          waitEntry = waitEntries.front();
          waitEntries.pop_front();
        }
      });
    if(waitEntry != nullptr) {
      NotifyWaitEntry(*waitEntry);
    }
  }

  inline void TimedConditionVariable::notify_all() {
    auto pendingWaitEntries = std::deque<WaitEntry*>();
    m_waitEntries.With(
      [&] (auto& waitEntries) {
        waitEntries.swap(pendingWaitEntries);
      });
    for(auto waitEntry : pendingWaitEntries) {
      NotifyWaitEntry(*waitEntry);
    }
  }

  inline void TimedConditionVariable::NotifyWaitEntry(WaitEntry& waitEntry) {
    auto isTimerStarted = waitEntry.m_isTimerStarted.exchange(true);
    auto isWaiting = waitEntry.m_isWaiting.exchange(false);
    if(isTimerStarted) {
      waitEntry.m_timer.Cancel();
    }
    if(isWaiting) {
      waitEntry.m_timerResult.GetEval().SetResult();
      waitEntry.m_condition.notify_one();
    }
  }
}

#endif
