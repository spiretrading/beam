#ifndef BEAM_TIMED_CONDITION_VARIABLE_HPP
#define BEAM_TIMED_CONDITION_VARIABLE_HPP
#include <atomic>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Threading/TimeoutException.hpp"
#include "Beam/TimeService/LiveTimer.hpp"

namespace Beam {

  /**
   * Implements a condition variable that can wait for a specified time period.
   */
  class TimedConditionVariable {
    public:

      /** Constructs a TimedConditionVariable. */
      TimedConditionVariable() = default;

      /**
       * Suspends the current Routine until a notification is received.
       * @param lock The lock synchronizing the notification event.
       */
      template<typename... Lock>
      void wait(Lock&... lock);

      /**
       * Suspends the current Routine for a specified amount of time.
       * @param duration The amount of time to wait.
       * @param lock The lock synchronizing the notification event.
       */
      template<typename... Lock>
      void timed_wait(
        boost::posix_time::time_duration duration, Lock&... lock);

      /** Triggers a notification event for a single suspended Routine. */
      void notify_one();

      /** Triggers a notification event for all suspended Routine. */
      void notify_all();

    private:
      struct WaitEntry {
        std::atomic_bool m_is_waiting;
        std::atomic_bool m_is_timer_started;
        LiveTimer m_timer;
        Async<void> m_timer_result;
        ConditionVariable m_condition;

        WaitEntry();
        WaitEntry(boost::posix_time::time_duration duration);
      };
      Sync<std::deque<WaitEntry*>> m_wait_entries;

      TimedConditionVariable(const TimedConditionVariable&) = delete;
      TimedConditionVariable& operator =(
        const TimedConditionVariable&) = delete;
      void notify_wait_entry(WaitEntry& wait_entry);
  };

  inline TimedConditionVariable::WaitEntry::WaitEntry()
    : m_is_waiting(true),
      m_is_timer_started(false),
      m_timer(boost::posix_time::seconds(0)) {}

  inline TimedConditionVariable::WaitEntry::WaitEntry(
    boost::posix_time::time_duration duration)
    : m_is_waiting(true),
      m_is_timer_started(false),
      m_timer(duration) {}

  template<typename... Lock>
  void TimedConditionVariable::wait(Lock&... lock) {
    auto wait_entry = WaitEntry();
    m_wait_entries.with([&] (auto& wait_entries) {
      wait_entries.push_back(&wait_entry);
    });
    wait_entry.m_condition.wait(lock...);
  }

  template<typename... Lock>
  void TimedConditionVariable::timed_wait(
      boost::posix_time::time_duration duration, Lock&... lock) {
    auto wait_entry = WaitEntry(duration);
    m_wait_entries.with([&] (auto& wait_entries) {
      wait_entries.push_back(&wait_entry);
    });
    auto wait_routine = spawn([&] {
      wait_entry.m_timer.start();
      auto is_timer_started = wait_entry.m_is_timer_started.exchange(true);
      if(is_timer_started) {
        wait_entry.m_timer.cancel();
        return;
      }
      wait_entry.m_timer.wait();
      auto is_waiting = wait_entry.m_is_waiting.exchange(false);
      if(is_waiting) {
        m_wait_entries.with([&] (auto& wait_entries) {
          auto i =
            std::find(wait_entries.begin(), wait_entries.end(), &wait_entry);
          if(i != wait_entries.end()) {
            wait_entries.erase(i);
          }
        });
        wait_entry.m_timer_result.get_eval().set_exception(TimeoutException());
        wait_entry.m_condition.notify_one();
      }
    });
    wait_entry.m_condition.wait(lock...);
    Beam::wait(wait_routine);
    wait_entry.m_timer_result.get();
  }

  inline void TimedConditionVariable::notify_one() {
    auto wait_entry = static_cast<WaitEntry*>(nullptr);
    m_wait_entries.with([&] (auto& wait_entries) {
      if(wait_entries.empty()) {
        wait_entry = nullptr;
      } else {
        wait_entry = wait_entries.front();
        wait_entries.pop_front();
      }
    });
    if(wait_entry) {
      notify_wait_entry(*wait_entry);
    }
  }

  inline void TimedConditionVariable::notify_all() {
    auto pending_wait_entries = std::deque<WaitEntry*>();
    m_wait_entries.with([&] (auto& wait_entries) {
      wait_entries.swap(pending_wait_entries);
    });
    for(auto wait_entry : pending_wait_entries) {
      notify_wait_entry(*wait_entry);
    }
  }

  inline void TimedConditionVariable::notify_wait_entry(WaitEntry& wait_entry) {
    auto is_timer_started = wait_entry.m_is_timer_started.exchange(true);
    auto is_waiting = wait_entry.m_is_waiting.exchange(false);
    if(is_timer_started) {
      wait_entry.m_timer.cancel();
    }
    if(is_waiting) {
      wait_entry.m_timer_result.get_eval().set();
      wait_entry.m_condition.notify_one();
    }
  }
}

#endif
