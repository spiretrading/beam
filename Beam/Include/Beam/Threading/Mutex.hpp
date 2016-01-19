#ifndef BEAM_MUTEX_HPP
#define BEAM_MUTEX_HPP
#include <deque>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class Mutex
      \brief Implements a mutex that suspends the current Routine.
   */
  class Mutex : private boost::noncopyable {
    public:

      //! Constructs a Mutex.
      Mutex();

      ~Mutex();

      //! Locks this Mutex.
      void lock();

      //! Tries to locks this Mutex.
      bool try_lock();

      //! Unlocks this Mutex.
      void unlock();

    private:
      boost::mutex m_mutex;
      int m_counter;
      std::deque<Routines::Routine*> m_suspendedRoutines;
  };

  inline Mutex::Mutex()
      : m_counter(0) {}

  inline Mutex::~Mutex() {
    assert(m_counter == 0);
  }

  inline void Mutex::lock() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    ++m_counter;
    if(m_counter > 1) {
      m_suspendedRoutines.push_back(&Routines::GetCurrentRoutine());
      Routines::GetCurrentRoutine().PendingSuspend();
      auto release = Release(lock);
      Routines::Suspend();
    }
  }

  inline bool Mutex::try_lock() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(m_counter > 0) {
      return false;
    }
    m_counter = 1;
    return true;
  }

  inline void Mutex::unlock() {
    Routines::Routine* routine;
    {
      boost::lock_guard<boost::mutex> lock(m_mutex);
      --m_counter;
      if(m_counter == 0) {
        return;
      }
      routine = m_suspendedRoutines.front();
      m_suspendedRoutines.pop_front();
    }
    Routines::Resume(routine);
  }
}
}

#endif
