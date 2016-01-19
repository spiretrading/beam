#ifndef BEAM_GILLOCK_HPP
#define BEAM_GILLOCK_HPP
#include <boost/noncopyable.hpp>
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
      GilLock() = default;

      //! Acquired the Python global interpreter lock.
      void lock();

      //! Releases the Python global interpreter lock.
      void unlock();

    private:
      bool m_hasThread;
      PyGILState_STATE m_state;
  };

  inline void GilLock::lock() {
    m_state = PyGILState_Ensure();
  }

  inline void GilLock::unlock() {
    PyGILState_Release(m_state);
  }
}
}

#endif
