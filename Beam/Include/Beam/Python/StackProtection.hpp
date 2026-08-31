#ifndef BEAM_PYTHON_STACK_PROTECTION_HPP
#define BEAM_PYTHON_STACK_PROTECTION_HPP
#include <pybind11/pybind11.h>
#include "Beam/Routines/ExternalRoutine.hpp"
#include "Beam/Routines/ScheduledRoutine.hpp"

namespace Beam::Python {

  /** Tells Python which stack it is currently running on. */
  inline void update_stack_protection() noexcept {
#if PY_VERSION_HEX >= 0x030E0000
    auto state = PyThreadState_GetUnchecked();
    if(!state) {
      return;
    }
    auto routine = dynamic_cast<ScheduledRoutine*>(
      Beam::Details::CurrentRoutineGlobal::get());
    if(routine && routine->get_stack_base()) {
      if(PyUnstable_ThreadState_SetStackProtection(state,
          routine->get_stack_base(), routine->get_stack_extent()) == 0) {
        return;
      }
      PyErr_Clear();
    }
    PyUnstable_ThreadState_ResetStackProtection(state);
#endif
  }

  /** Tells Python that it is running on the stack of the thread again. */
  inline void reset_stack_protection() noexcept {
#if PY_VERSION_HEX >= 0x030E0000
    if(auto state = PyThreadState_GetUnchecked()) {
      PyUnstable_ThreadState_ResetStackProtection(state);
    }
#endif
  }
}

#endif
