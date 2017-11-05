#ifndef BEAM_SUSPENDEDROUTINEQUEUE_INL
#define BEAM_SUSPENDEDROUTINEQUEUE_INL
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Routines/Routine.hpp"

namespace Beam {
namespace Routines {
  template<typename... Lock>
  void Suspend(Out<Threading::Sync<SuspendedRoutineQueue>> suspendedRoutines,
      Lock&... lock) {
    SuspendedRoutineNode currentRoutine;
    Threading::With(*suspendedRoutines,
      [&] (auto& queue) {
        currentRoutine.m_routine->PendingSuspend();
        queue.push_back(currentRoutine);
      });
    auto releases = std::make_tuple(Threading::Release(lock)...);
    Suspend();
  }

  template<typename... Lock>
  void ResumeFront(Out<Threading::Sync<SuspendedRoutineQueue>>
      suspendedRoutines) {
    auto routine = Threading::With(*suspendedRoutines,
      [] (auto& suspendedRoutines) {
        if(suspendedRoutines.empty()) {
          return static_cast<Routine*>(nullptr);
        }
        auto routine = suspendedRoutines.front().m_routine;
        suspendedRoutines.pop_front();
        return routine;
      });
    Routines::Resume(routine);
  }

  template<typename... Lock>
  void Resume(Out<Threading::Sync<SuspendedRoutineQueue>> suspendedRoutines) {
    SuspendedRoutineQueue resumedRoutines;
    Threading::With(*suspendedRoutines,
      [&] (auto& suspendedRoutines) {
        resumedRoutines.swap(suspendedRoutines);
      });
    for(auto& routine : resumedRoutines) {
      Resume(routine.m_routine);
    }
  }

  inline SuspendedRoutineNode::SuspendedRoutineNode()
      : m_routine{&GetCurrentRoutine()} {}
}
}

#endif
