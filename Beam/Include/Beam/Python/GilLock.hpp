#ifndef BEAM_PYTHON_GIL_LOCK_HPP
#define BEAM_PYTHON_GIL_LOCK_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /** Acquires Python's global interpreter lock. */
  class GilLock {
    public:

      /** Locks the GIL. */
      GilLock() noexcept;

      ~GilLock();

    private:
      bool m_has_gil;
      PyGILState_STATE m_state;

      GilLock(const GilLock&) = delete;
      GilLock& operator =(const GilLock&) = delete;
  };

  /**
   * Returns <code>true</code> iff this thread has the global interpreter
   * lock.
   */
  inline bool has_gil() {
    return PyGILState_Check() != 0;
  }

  inline GilLock::GilLock() noexcept {
    m_has_gil = has_gil();
    if(!m_has_gil) {
      m_state = PyGILState_Ensure();
    }
  }

  inline GilLock::~GilLock() noexcept {
    if(!m_has_gil) {
      PyGILState_Release(m_state);
    }
  }
}

#endif
