#ifndef BEAM_PYTHON_GIL_LOCK_HPP
#define BEAM_PYTHON_GIL_LOCK_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /** Acquires Python's global interpreter lock. */
  class GilLock {
    public:

      /** Locks the GIL/ */
      GilLock();

      ~GilLock();

    private:
      bool m_hasGil;
      PyGILState_STATE m_state;

      GilLock(const GilLock&) = delete;
      GilLock& operator =(const GilLock&) = delete;
  };

  /**
   * Returns <code>true</code> iff this thread has the global interpreter
   * lock.
   */
  inline bool HasGil() {
    return PyGILState_Check() != 0;
  }

  inline GilLock::GilLock() {
    m_hasGil = HasGil();
    if(!m_hasGil) {
      m_state = PyGILState_Ensure();
    }
  }

  inline GilLock::~GilLock() {
    if(!m_hasGil) {
      PyGILState_Release(m_state);
    }
  }
}

#endif
