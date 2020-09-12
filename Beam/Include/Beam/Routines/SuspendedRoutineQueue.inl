#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#define BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Routines/Routine.hpp"

namespace Beam::Routines {
  template<typename... Lock>
  void Suspend(Out<SuspendedRoutineQueue> suspendedRoutines, Lock&... lock) {
    auto currentRoutine = SuspendedRoutineNode();
    currentRoutine.m_routine->PendingSuspend();
    suspendedRoutines->push_back(currentRoutine);
    auto releases = std::make_tuple(Threading::Release(lock)...);
    Suspend();
  }

  template<typename... Lock>
  void ResumeFront(Out<SuspendedRoutineQueue> suspendedRoutines) {
    if(suspendedRoutines->empty()) {
      return;
    }
    auto routine = suspendedRoutines->front().m_routine;
    suspendedRoutines->pop_front();
    Routines::Resume(routine);
  }

  template<typename... Lock>
  void Resume(Out<SuspendedRoutineQueue> suspendedRoutines) {
    auto resumedRoutines = SuspendedRoutineQueue();
    resumedRoutines.swap(*suspendedRoutines);
    for(auto& routine : resumedRoutines) {
      Resume(routine.m_routine);
    }
  }

  inline SuspendedRoutineNode::SuspendedRoutineNode()
    : m_routine(&GetCurrentRoutine()) {}
}

#endif
