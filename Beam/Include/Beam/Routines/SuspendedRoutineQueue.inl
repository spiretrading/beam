#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#define BEAM_SUSPENDED_ROUTINE_QUEUE_INL
#include <type_traits>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"

namespace Beam {
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
    Releaser<std::remove_cvref_t<R1>, std::remove_cvref_t<R2>>;

  template<typename Head>
  auto reverse_release(Head& head) {
    return release(head);
  }

  template<typename Head, typename... Tail>
  auto reverse_release(Head& head, Tail&... tail) {
    return Releaser(reverse_release(tail...), reverse_release(head));
  }

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
