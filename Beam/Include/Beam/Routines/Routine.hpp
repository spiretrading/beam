#ifndef BEAM_ROUTINE_HPP
#define BEAM_ROUTINE_HPP
#include <atomic>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <vector>
#include "Beam/Routines/Async.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/DllExport.hpp"
#include <boost/thread.hpp>

namespace Beam {
namespace Details {
  struct BEAM_EXPORT_DLL CurrentRoutineGlobal {
    static Routine*& get() {
      static thread_local auto value = static_cast<Routine*>(nullptr);
      return value;
    }
  };

  struct BEAM_EXPORT_DLL NextId {
    static std::atomic_uint64_t& get() {
      static auto m_value = std::atomic_uint64_t();
      return m_value;
    }
  };
}

  /** Encapsulates a single sub-routine spawned by a Scheduler. */
  class Routine {
    public:

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
      Routine() noexcept;

      virtual ~Routine();

      /** Returns this Routine's identifier. */
      Id get_id() const;

      /** Returns the State of this Routine. */
      State get_state() const;

      /**
       * Waits for the completion of this Routine.
       * @param result Used to signal completion of this Routine.
       */
      void wait(Eval<void> result);

      /** Implements the body of this Routine. */
      virtual void execute() = 0;

      /** Defers execution of this Routine so that another may run. */
      virtual void defer() = 0;

      /** Marks this Routine as about to enter a suspended State. */
      virtual void pending_suspend() = 0;

      /** Suspends execution of this Routine until it is resumed. */
      virtual void suspend() = 0;

      /** Resumes execution of a Routine. */
      virtual void resume() = 0;

    protected:

      /** Sets the State. */
      void set(State state);

    private:
      using WaitResults = std::vector<Eval<void>>;
      friend class Mutex;
      friend class RecursiveMutex;
      friend void defer();
      friend void suspend();
      template<typename HeadLock, typename... TailLocks>
      friend void suspend(Out<Routine*>, HeadLock&, TailLocks&...);
      template<typename Container, typename... Lock>
      friend void suspend(Out<Sync<Container>> suspendedRoutines,
        Lock&... lock);
      friend void resume(Routine*&);
      template<typename Container>
      friend void resume(Out<Sync<Container>>);
      State m_state;
      Id m_id;
      Sync<WaitResults> m_wait_results;
  };

  /** Returns the currently executing Routine. */
  Routine& get_current_routine();

  /** Defers the currently executing Routine. */
  inline void defer() {
    get_current_routine().defer();
  }

  /**
   * Waits for a Routine to complete.
   * @param id The id of the Routine to wait for.
   */
  void wait(Routine::Id id);

  /** Suspends the currently running Routine. */
  inline void suspend() {
    get_current_routine().suspend();
  }

  /**
   * Suspends the currently running Routine.
   * @param routine Stores the Routine being suspended.
   */
  inline void Suspend(Out<Routine*> routine) {
    *routine = &get_current_routine();
    suspend();
  }

  /**
   * Suspends the currently running Routine.
   * @param routine Stores the Routine being suspended.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename HeadLock, typename... TailLocks>
  void suspend(Out<Routine*> routine, HeadLock& head, TailLocks&... tail) {
    *routine = &get_current_routine();
    get_current_routine().pending_suspend();
    auto releases = std::tuple(release(head), release(tail)...);
    suspend();
  }

  /**
   * Resumes execution of a suspended Routine.
   * @param routine The Routine to resume.
   */
  inline void resume(Routine*& routine) {
    if(!routine) {
      return;
    }
    auto initial_routine = routine;
    routine = nullptr;
    initial_routine->resume();
  }

  inline Routine::Routine() noexcept
    : m_id(++Details::NextId::get()),
      m_state(State::PENDING) {}

  inline Routine::~Routine() {
    with(m_wait_results, [&] (auto& results) {
      for(auto& result : results) {
        result.set();
      }
      results.clear();
    });
    assert(m_state == State::COMPLETE || m_state == State::PENDING);
  }

  inline Routine::Id Routine::get_id() const {
    return m_id;
  }

  inline Routine::State Routine::get_state() const {
    return m_state;
  }

  inline void Routine::wait(Eval<void> result) {
    with(m_wait_results, [&] (auto& results) {
      results.push_back(std::move(result));
    });
  }

  inline void Routine::set(State state) {
    m_state = state;
  }

  inline void Routine::defer() {}

  inline void Routine::pending_suspend() {}

  inline void Routine::suspend() {}

  inline void Routine::resume() {}
}

#include "Beam/Routines/ExternalRoutine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.inl"
#include "Beam/Routines/Async.inl"
#include "Beam/Routines/Scheduler.hpp"

#endif
