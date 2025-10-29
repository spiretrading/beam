#ifndef BEAM_CONDITION_VARIABLE_HPP
#define BEAM_CONDITION_VARIABLE_HPP
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {

  /** Implements a condition variable that suspends the current Routine. */
  class ConditionVariable {
    public:

      /** Constructs a ConditionVariable. */
      ConditionVariable() = default;

      /**
       * Suspends the current Routine until a notification is received.
       * @param lock The lock synchronizing the notification event.
       */
      template<typename... Lock>
      void wait(Lock&... lock);

      /** Triggers a notification event for a single suspended Routine. */
      void notify_one();

      /** Triggers a notification event for all suspended Routine. */
      void notify_all();

    private:
      boost::mutex m_mutex;
      SuspendedRoutineQueue m_suspended_routines;

      ConditionVariable(const ConditionVariable&) = delete;
      ConditionVariable& operator =(const ConditionVariable&) = delete;
  };

  template<typename... Lock>
  void ConditionVariable::wait(Lock&... lock) {
    auto self_lock = boost::unique_lock(m_mutex);
    suspend(out(m_suspended_routines), lock..., self_lock);
  }

  inline void ConditionVariable::notify_one() {
    auto lock = boost::lock_guard(m_mutex);
    resume_front(out(m_suspended_routines));
  }

  inline void ConditionVariable::notify_all() {
    auto lock = boost::lock_guard(m_mutex);
    resume(out(m_suspended_routines));
  }
}

#include "Beam/Routines/SuspendedRoutineQueue.inl"

#endif
