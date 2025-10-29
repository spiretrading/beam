#ifndef BEAM_PYTHON_GIL_RELEASE_HPP
#define BEAM_PYTHON_GIL_RELEASE_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "Beam/Python/GilLock.hpp"

namespace Beam::Python {

  /** Releases Python's global interpreter lock. */
  class GilRelease {
    public:

      /** Releases the GIL. */
      GilRelease() noexcept;

      ~GilRelease();

    private:
      bool m_has_gil;
      PyThreadState* m_state;

      GilRelease(const GilRelease&) = delete;
      GilRelease& operator =(const GilRelease&) = delete;
  };

  /**
   * Creates a shared_ptr with a custom deleter that releases the GIL during
   * destruction.
   * @param args The arguments to forward to T's constructor.
   * @return A shared_ptr to T with a GIL-releasing deleter.
   */
  template<typename T, typename... Args>
  auto make_python_shared(Args&&... args) {
    return std::shared_ptr<T>(
      new T(std::forward<Args>(args)...), [] (auto* object) {
        auto release = GilRelease();
        delete object;
      });
  }

  inline GilRelease::GilRelease() noexcept {
    m_has_gil = has_gil();
    if(m_has_gil) {
      m_state = PyEval_SaveThread();
    }
  }

  inline GilRelease::~GilRelease() {
    if(m_has_gil) {
      PyEval_RestoreThread(m_state);
    }
  }
}

#endif
