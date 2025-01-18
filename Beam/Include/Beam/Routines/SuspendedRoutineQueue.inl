#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#define BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#include <type_traits>
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Routines/Routine.hpp"

namespace Beam::Routines {
  template<typename T1, typename T2>
  struct Releaser {
    T1 m_head;
    T2 m_tail;

    template<typename R1, typename R2>
    Releaser(R1&& head, R2&& tail)
      : m_head(std::forward<R1>(head)),
        m_tail(std::forward<R2>(tail)) {}
  };

  template<typename R1, typename R2>
  Releaser(R1&&, R2&&) ->
    Releaser<std::remove_reference_t<R1>, std::remove_reference_t<R2>>;

  template<typename Head>
  auto ReverseRelease(Head& head) {
    return Threading::Release(head);
  }

  template<typename Head, typename... Tail>
  auto ReverseRelease(Head& head, Tail&... tail) {
    return Releaser(ReverseRelease(tail...), ReverseRelease(head));
  }

  template<typename... Lock>
  void Suspend(Out<SuspendedRoutineQueue<>> suspendedRoutines, Lock&... lock) {
    auto currentRoutine = SuspendedRoutineNode();
    currentRoutine.m_routine->PendingSuspend();
    suspendedRoutines->push_back(currentRoutine);
    auto releases = ReverseRelease(lock...);
    Suspend();
  }

  template<typename T, typename... Lock>
  void Suspend(
      Out<SuspendedRoutineQueue<T>> suspendedRoutines, T key, Lock&... lock) {
    auto currentRoutine = SuspendedRoutineNode<T>();
    currentRoutine.m_key = std::move(key);
    currentRoutine.m_routine->PendingSuspend();
    suspendedRoutines->push_back(currentRoutine);
    auto releases = ReverseRelease(lock...);
    Suspend();
  }

  template<typename T>
  void ResumeFront(Out<SuspendedRoutineQueue<T>> suspendedRoutines) {
    if(suspendedRoutines->empty()) {
      return;
    }
    auto routine = suspendedRoutines->front().m_routine;
    suspendedRoutines->pop_front();
    Routines::Resume(routine);
  }

  template<typename T, typename Lock>
  void ResumeFirstMatch(Out<SuspendedRoutineQueue<T>> suspendedRoutines,
      const T& key, Lock& lock) {
    for(auto i = suspendedRoutines->begin();
        i != suspendedRoutines->end(); ++i) {
      if(i->m_key == key) {
        auto suspendedRoutine = i->m_routine;
        suspendedRoutines->erase(i);
        auto release = Threading::Release(lock);
        Routines::Resume(suspendedRoutine);
        break;
      }
    }
  }

  template<typename T, typename Lock>
  void ResumeAllMatches(Out<SuspendedRoutineQueue<T>> suspendedRoutines,
      const T& key, Lock& lock) {
    auto i = suspendedRoutines->begin();
    while(i != suspendedRoutines->end()) {
      if(i->m_key == key) {
        auto suspendedRoutine = i->m_routine;
        i = suspendedRoutines->erase(i);
        auto release = Threading::Release(lock);
        Routines::Resume(suspendedRoutine);
      } else {
        ++i;
      }
    }
  }

  template<typename T>
  void Resume(Out<SuspendedRoutineQueue<T>> suspendedRoutines) {
    auto resumedRoutines = SuspendedRoutineQueue<T>();
    resumedRoutines.swap(*suspendedRoutines);
    for(auto& routine : resumedRoutines) {
      Resume(routine.m_routine);
    }
  }

  template<typename T>
  SuspendedRoutineNode<T>::SuspendedRoutineNode()
    : m_routine(&GetCurrentRoutine()) {}
}

#endif
