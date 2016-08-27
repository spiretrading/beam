#ifndef BEAM_CONDITIONVARIABLE_HPP
#define BEAM_CONDITIONVARIABLE_HPP
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam {
namespace Threading {

  /*! \class ConditionVariable
      \brief Implements a condition variable that suspends the current Routine.
   */
  class ConditionVariable : private boost::noncopyable {
    public:

      //! Constructs a ConditionVariable.
      ConditionVariable() = default;

      //! Suspends the current Routine until a notification is received.
      /*!
        \param lock The lock synchronizing the notification event.
      */
      template<typename... Lock>
      void wait(Lock&... lock);

      //! Triggers a notification event for a single suspended Routine.
      void notify_one();

      //! Triggers a notification event for all suspended Routine.
      void notify_all();

    private:
      Sync<Routines::SuspendedRoutineQueue> m_suspendedRoutines;
  };

  template<typename... Lock>
  void ConditionVariable::wait(Lock&... lock) {
    Routines::Suspend(Store(m_suspendedRoutines), lock...);
  }

  inline void ConditionVariable::notify_one() {
    Routines::ResumeFront(Store(m_suspendedRoutines));
  }

  inline void ConditionVariable::notify_all() {
    Routines::Resume(Store(m_suspendedRoutines));
  }
}
}

#include "Beam/Routines/SuspendedRoutineQueue.inl"

#endif
