#ifndef BEAM_RECURSIVE_MUTEX_HPP
#define BEAM_RECURSIVE_MUTEX_HPP
#include <cstdint>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/LockRelease.hpp"

namespace Beam {

  /** Implements a recursive_mutex that suspends the current Routine. */
  class RecursiveMutex {
    public:

      /** Constructs a RecursiveMutex. */
      RecursiveMutex() noexcept;

#ifndef NDEBUG
      ~RecursiveMutex();
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
      int m_depth;
      Routine* m_owner;
      SuspendedRoutineQueue m_routines;

      RecursiveMutex(const RecursiveMutex&) = delete;
      RecursiveMutex& operator =(const RecursiveMutex&) = delete;
  };

  inline RecursiveMutex::RecursiveMutex() noexcept
    : m_counter(0),
      m_depth(0),
      m_owner(nullptr) {}

#ifndef NDEBUG
  inline RecursiveMutex::~RecursiveMutex() {
    assert(m_counter == 0);
  }
#endif

  inline void RecursiveMutex::lock() {
    auto current = SuspendedRoutineNode();
    auto lock = boost::unique_lock(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      if(current.m_routine != m_owner) {
        m_routines.push_back(current);
        current.m_routine->pending_suspend();
        auto releaser = release(lock);
        suspend();
      }
    }
    ++m_depth;
    m_owner = current.m_routine;
  }

  inline bool RecursiveMutex::try_lock() {
    auto current = &get_current_routine();
    auto lock = boost::lock_guard(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      if(current != m_owner) {
        --m_counter;
        return false;
      }
    }
    ++m_depth;
    m_owner = current;
    return true;
  }

  inline void RecursiveMutex::unlock() {
    auto lock = boost::lock_guard(m_mutex);
    --m_depth;
    if(m_depth == 0) {
      m_owner = nullptr;
    }
    --m_counter;
    if(m_counter > 0) {
      if(m_depth == 0) {
        if(m_routines.empty()) {
          return;
        }
        auto routine = m_routines.front().m_routine;
        m_routines.pop_front();
        resume(routine);
      }
    }
  }
}

#endif
