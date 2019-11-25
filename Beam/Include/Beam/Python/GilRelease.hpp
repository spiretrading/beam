#ifndef BEAM_GIL_RELEASE_HPP
#define BEAM_GIL_RELEASE_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/GilLock.hpp"

namespace Beam::Python {

  /** Releases Python's global interpreter lock. */
  class GilRelease {
    public:

      /** Releases the GIL. */
      GilRelease();

      ~GilRelease();

    private:
      bool m_hasGil;
      PyThreadState* m_state;

      GilRelease(const GilRelease&) = delete;
      GilRelease& operator =(const GilRelease&) = delete;
  };

  inline GilRelease::GilRelease() {
    m_hasGil = HasGil();
    if(m_hasGil) {
      m_state = PyEval_SaveThread();
    }
  }

  inline GilRelease::~GilRelease() {
    if(m_hasGil) {
      PyEval_RestoreThread(m_state);
    }
  }
}

#endif
