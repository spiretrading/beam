#ifndef BEAM_MUTEX_HPP
#define BEAM_MUTEX_HPP
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /** Implements a mutex that suspends the current Routine. */
  class Mutex {
    public:

      /** Constructs a Mutex. */
      Mutex();

      ~Mutex();

      /** Locks this Mutex. */
      void lock();

      /** Tries to locks this Mutex. */
      bool try_lock();

      /** Unlocks this Mutex. */
      void unlock();

    private:
      boost::mutex m_mutex;
      int m_counter;
      Routines::SuspendedRoutineQueue m_suspendedRoutines;

      Mutex(const Mutex&) = delete;
      Mutex& operator =(const Mutex&) = delete;
  };

  inline Mutex::Mutex()
    : m_counter(0) {}

  inline Mutex::~Mutex() {
    assert(m_counter == 0);
  }

  inline void Mutex::lock() {
    auto lock = boost::unique_lock(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      auto currentRoutine = Routines::SuspendedRoutineNode();
      m_suspendedRoutines.push_back(currentRoutine);
      currentRoutine.m_routine->PendingSuspend();
      auto release = Release(lock);
      Routines::Suspend();
    }
  }

  inline bool Mutex::try_lock() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_counter > 0) {
      return false;
    }
    m_counter = 1;
    return true;
  }

  inline void Mutex::unlock() {
    auto routine = static_cast<Routines::Routine*>(nullptr);
    {
      auto lock = boost::lock_guard(m_mutex);
      --m_counter;
      if(m_counter == 0) {
        return;
      }
      routine = m_suspendedRoutines.front().m_routine;
      m_suspendedRoutines.pop_front();
    }
    Routines::Resume(routine);
  }
}

#endif
