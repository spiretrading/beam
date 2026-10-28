#ifndef BEAM_PYTHON_GIL_LOCK_HPP
#define BEAM_PYTHON_GIL_LOCK_HPP
#include <exception>
#include <pybind11/pybind11.h>
#include "Beam/Python/StackProtection.hpp"

namespace Beam::Python {

  /** Acquires Python's global interpreter lock. */
  class GilLock {
    public:

      /** Locks the GIL. */
      GilLock() noexcept;

      ~GilLock();

    private:
      PyThreadState* m_state;
      PyThreadState* m_binding_state;
      pybind11::detail::loader_life_support* m_loader;

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

  inline GilLock::GilLock() noexcept
      : m_state(nullptr),
        m_binding_state(nullptr),
        m_loader(nullptr) {
    if(!has_gil()) {
      m_state = PyThreadState_New(PyInterpreterState_Main());
      if(!m_state) {
        std::terminate();
      }
      PyEval_RestoreThread(m_state);
      update_stack_protection();
      auto& internals = pybind11::detail::get_internals();
      m_binding_state = internals.tstate.get();
      m_loader = internals.loader_life_support_tls.get();
      internals.tstate = m_state;
      internals.loader_life_support_tls = nullptr;
    } else {
      update_stack_protection();
    }
  }

  inline GilLock::~GilLock() noexcept {
    if(m_state) {
      PyThreadState_Clear(m_state);
      auto& internals = pybind11::detail::get_internals();
      internals.tstate = m_binding_state;
      internals.loader_life_support_tls = m_loader;
      reset_stack_protection();
      PyThreadState_DeleteCurrent();
    }
  }
}

#endif
