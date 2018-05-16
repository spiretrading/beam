#ifndef BEAM_GILLOCK_HPP
#define BEAM_GILLOCK_HPP
#include <boost/noncopyable.hpp>
#include <ceval.h>
#include <pystate.h>
#include <pythread.h>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  /*! \class GilLock
      \brief Acquires Python's global interpreter lock.
   */
  class GilLock : boost::noncopyable {
    public:

      //! Constructs a GilLock.
      GilLock();

      //! Acquired the Python global interpreter lock.
      void lock();

      //! Releases the Python global interpreter lock.
      void unlock();

    private:
      bool m_hasGil;
      PyGILState_STATE m_state;
  };

  //! Returns <code>true</code> iff this thread has the global interpreter lock.
  inline bool HasGil() {
    return PyGILState_Check() != 0;
  }

  inline GilLock::GilLock() = default;

  inline void GilLock::lock() {
    m_hasGil = HasGil();
    if(!m_hasGil) {
      m_state = PyGILState_Ensure();
    }
  }

  inline void GilLock::unlock() {
    if(!m_hasGil) {
      PyGILState_Release(m_state);
    }
  }
}
}

#endif
