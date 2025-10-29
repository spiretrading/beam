#ifndef BEAM_MUTEX_HPP
#define BEAM_MUTEX_HPP
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/LockRelease.hpp"

namespace Beam {

  /** Implements a mutex that suspends the current Routine. */
  class Mutex {
    public:

      /** Constructs a Mutex. */
      Mutex() noexcept;

#ifndef NDEBUG
      ~Mutex();
#endif

      /** Locks this Mutex. */
      void lock();

      /** Tries to locks this Mutex. */
      bool try_lock();

      /** Unlocks this Mutex. */
      void unlock();

    private:
      boost::mutex m_mutex;
      int m_counter;
      SuspendedRoutineQueue m_suspended_routines;

      Mutex(const Mutex&) = delete;
      Mutex& operator =(const Mutex&) = delete;
  };

  inline Mutex::Mutex() noexcept
    : m_counter(0) {}

#ifndef NDEBUG
  inline Mutex::~Mutex() {
    assert(m_counter == 0);
  }
#endif

  inline void Mutex::lock() {
    auto lock = boost::unique_lock(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      auto routine = SuspendedRoutineNode();
      m_suspended_routines.push_back(routine);
      routine.m_routine->pending_suspend();
      auto releaser = release(lock);
      suspend();
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
    auto routine = static_cast<Routine*>(nullptr);
    {
      auto lock = boost::lock_guard(m_mutex);
      --m_counter;
      if(m_counter == 0) {
        return;
      }
      routine = m_suspended_routines.front().m_routine;
      m_suspended_routines.pop_front();
    }
    resume(routine);
  }
}

#endif
