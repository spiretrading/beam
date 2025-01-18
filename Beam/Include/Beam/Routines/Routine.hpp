#ifndef BEAM_ROUTINE_HPP
#define BEAM_ROUTINE_HPP
#include <atomic>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <vector>
#include "Beam/Routines/Async.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Routines {

  /** Encapsulates a single sub-routine spawned by a Scheduler. */
  class Routine {
    public:

      /** Indicates whether a routine is currently running on this thread. */
      static inline thread_local auto m_isInsideRoutine = false;

      /** Lists the states a Routine can be in. */
      enum class State {

        /** The Routine is waiting to be run. */
        PENDING,

        /** The Routine is currently running. */
        RUNNING,

        /** The Routine is pending a suspend. */
        PENDING_SUSPEND,

        /** The Routine is suspended. */
        SUSPENDED,

        /** The Routine has completed. */
        COMPLETE
      };

      /** The type used to identify a Routine. */
      using Id = std::uint64_t;

      /** Constructs a Routine. */
      Routine();

      virtual ~Routine();

      /** Returns this Routine's identifier. */
      Id GetId() const;

      /** Returns the State of this Routine. */
      State GetState() const;

      /**
       * Waits for the completion of this Routine.
       * @param result Used to signal completion of this Routine.
       */
      void Wait(Eval<void> result);

      /** Defers execution of this Routine so that another may run. */
      virtual void Defer() = 0;

      /** Marks this Routine as about to enter a suspended State. */
      virtual void PendingSuspend() = 0;

      /** Suspends execution of this Routine until it is resumed. */
      virtual void Suspend() = 0;

      /** Resumes execution of a Routine. */
      virtual void Resume() = 0;

    protected:

      /** A pointer to this thread's local current routine. */
      static inline thread_local auto m_currentRoutine =
        static_cast<Routine*>(nullptr);

      /** Sets the State. */
      void SetState(State state);

    private:
      using WaitResults = std::vector<Eval<void>>;
      friend Routine& GetCurrentRoutine();
      friend void Defer();
      friend void Suspend();
      template<typename HeadLock, typename... TailLocks>
      friend void Suspend(Out<Routine*>, HeadLock&, TailLocks&...);
      template<typename Container, typename... Lock>
      friend void Suspend(
        Out<Threading::Sync<Container>> suspendedRoutines, Lock&... lock);
      friend void Resume(Routine*&);
      template<typename Container>
      friend void Resume(Out<Threading::Sync<Container>>);
      static inline auto m_next_id = std::atomic_uint64_t(0);
      State m_state;
      Id m_id;
      Threading::Sync<WaitResults> m_waitResults;
  };

  /** Returns the currently executing Routine. */
  Routine& GetCurrentRoutine();

  /** Defers the currently executing Routine. */
  inline void Defer() {
    GetCurrentRoutine().Defer();
  }

  /**
   * Waits for a Routine to complete.
   * @param id The id of the Routine to wait for.
   */
  void Wait(Routine::Id id);

  /** Suspends the currently running Routine. */
  inline void Suspend() {
    GetCurrentRoutine().Suspend();
  }

  /**
   * Suspends the currently running Routine.
   * @param suspendedRoutine Stores the Routine being suspended.
   */
  inline void Suspend(Out<Routine*> suspendedRoutine) {
    *suspendedRoutine = &GetCurrentRoutine();
    Suspend();
  }

  /**
   * Suspends the currently running Routine.
   * @param suspendedRoutine Stores the Routine being suspended.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename HeadLock, typename... TailLocks>
  void Suspend(Out<Routine*> suspendedRoutine,
      HeadLock& headLock, TailLocks&... tailLocks) {
    *suspendedRoutine = &GetCurrentRoutine();
    GetCurrentRoutine().PendingSuspend();
    auto releases = std::tuple(
      Threading::Release(headLock), Threading::Release(tailLocks)...);
    Suspend();
  }

  /**
   * Resumes execution of a suspended Routine.
   * @param routine The Routine to resume.
   */
  inline void Resume(Routine*& routine) {
    if(!routine) {
      return;
    }
    auto initialRoutine = routine;
    routine = nullptr;
    initialRoutine->Resume();
  }

  inline Routine::Routine()
    : m_id(++m_next_id),
      m_state(State::PENDING) {}

  inline Routine::~Routine() {
    Threading::With(m_waitResults, [&] (auto& waitResults) {
      for(auto& waitResult : waitResults) {
        waitResult.SetResult();
      }
      waitResults.clear();
    });
    assert(m_state == State::COMPLETE || m_state == State::PENDING);
  }

  inline Routine::Id Routine::GetId() const {
    return m_id;
  }

  inline Routine::State Routine::GetState() const {
    return m_state;
  }

  inline void Routine::Wait(Eval<void> result) {
    Threading::With(m_waitResults, [&] (auto& waitResults) {
      waitResults.push_back(std::move(result));
    });
  }

  inline void Routine::SetState(State state) {
    m_state = state;
  }

  inline void Routine::Defer() {}

  inline void Routine::PendingSuspend() {}

  inline void Routine::Suspend() {}

  inline void Routine::Resume() {}
}

#include "Beam/Routines/ExternalRoutine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.inl"
#include "Beam/Routines/Scheduler.hpp"

#endif
