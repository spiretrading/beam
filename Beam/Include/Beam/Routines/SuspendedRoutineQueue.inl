#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#define BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"

namespace Beam {
  template<typename... Lock>
  void suspend(Out<SuspendedRoutineQueue> routines, Lock&... lock) {
    auto current = SuspendedRoutineNode();
    current.m_routine->pending_suspend();
    routines->push_back(current);
    auto releases = reverse_release(lock...);
    suspend();
  }

  template<typename... Lock>
  void resume_front(Out<SuspendedRoutineQueue> routines) {
    if(routines->empty()) {
      return;
    }
    auto routine = routines->front().m_routine;
    routines->pop_front();
    resume(routine);
  }

  template<typename... Lock>
  void resume(Out<SuspendedRoutineQueue> routines) {
    auto resumed_routines = SuspendedRoutineQueue();
    resumed_routines.swap(*routines);
    for(auto& routine : resumed_routines) {
      resume(routine.m_routine);
    }
  }

  inline SuspendedRoutineNode::SuspendedRoutineNode() noexcept
    : m_routine(&get_current_routine()) {}
}

#endif
