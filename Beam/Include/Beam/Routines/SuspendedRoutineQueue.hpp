#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#define BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#include <type_traits>
#include <utility>
#include <boost/intrusive/list.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Routines.hpp"
#include "Beam/Routines/Routine.hpp"

namespace Beam::Routines::Details {
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

  /**
   * Stores some data that can be used as a key to identify a suspended routine.
   */
  template<typename T>
  struct SuspendedRoutineNodeKey {
    using Type = T;
    Type m_key;
  };

  /** Specialization for when no key is needed. */
  template<>
  struct SuspendedRoutineNodeKey<void> {};

  /** Stores an intrusive node to the current Routine. */
  template<typename T = void>
  struct SuspendedRoutineNode : public boost::intrusive::list_base_hook<>,
      public SuspendedRoutineNodeKey<T> {

    /** The suspended routine. */
    Routines::Routine* m_routine;

    /** Constructs a SuspendedRoutineNode representing the current Routine. */
    SuspendedRoutineNode();
  };

  /** An intrusive linked list of suspended Routines. */
  template<typename T = void>
  using SuspendedRoutineQueue = boost::intrusive::list<SuspendedRoutineNode<T>>;

  /**
   * Suspends the currently running Routine.
   * @param suspendedRoutines Stores the Routine being suspended.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename... Lock>
  void Suspend(Out<SuspendedRoutineQueue<>> suspendedRoutines, Lock&... lock) {
    auto currentRoutine = SuspendedRoutineNode();
    currentRoutine.m_routine->PendingSuspend();
    suspendedRoutines->push_back(currentRoutine);
    auto releases = ReverseRelease(lock...);
    Routines::Suspend();
  }

  /**
   * Suspends the currently running Routine.
   * @param suspendedRoutines Stores the Routine being suspended.
   * @param key The key to associated with the current routine.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename T, typename... Lock>
  void Suspend(
      Out<SuspendedRoutineQueue<T>> suspendedRoutines, T key, Lock&... lock) {
    auto currentRoutine = SuspendedRoutineNode<T>();
    currentRoutine.m_key = std::move(key);
    currentRoutine.m_routine->PendingSuspend();
    suspendedRoutines->push_back(currentRoutine);
    auto releases = ReverseRelease(lock...);
    Routines::Suspend();
  }

  /**
   * Resumes the first Routine in a queue of suspended Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   */
  template<typename T>
  void ResumeFront(Out<SuspendedRoutineQueue<T>> suspendedRoutines) {
    if(suspendedRoutines->empty()) {
      return;
    }
    auto routine = suspendedRoutines->front().m_routine;
    suspendedRoutines->pop_front();
    Routines::Resume(routine);
  }

  /**
   * Resumes all Routines found in a queue of suspended Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   */
  template<typename T>
  void Resume(Out<SuspendedRoutineQueue<T>> suspendedRoutines) {
    auto resumedRoutines = SuspendedRoutineQueue<T>();
    resumedRoutines.swap(*suspendedRoutines);
    while(!resumedRoutines.empty()) {
      auto resumedRoutine = &resumedRoutines.front();
      resumedRoutines.erase(resumedRoutines.begin());
      Resume(resumedRoutine->m_routine);
    }
  }

  /**
   * Resumes the first Routine found that matches a key in a queue of suspended
   * Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   * @param key The key to find.
   * @param lock The lock used to synchronize the suspended routines.
   */
  template<typename T, typename Lock>
  void ResumeFirstMatch(Out<SuspendedRoutineQueue<T>> suspendedRoutines,
      const T& key, Lock& lock) {
    for(auto i = suspendedRoutines->begin();
        i != suspendedRoutines->end(); ++i) {
      if(i->m_key == key) {
        auto suspendedRoutine = i->m_routine;
        suspendedRoutines->erase(i);
        auto release = Threading::Release(lock);
        auto isInsideRoutine = Routine::IsInsideRoutine();
        Routine::IsInsideRoutine() = false;
        Routines::Resume(suspendedRoutine);
        Routine::IsInsideRoutine() = isInsideRoutine;
        break;
      }
    }
  }

  /**
   * Resumes all Routines found that match a key in a queue of suspended
   * Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   * @param key The key to find.
   * @param lock The lock used to synchronize the suspended routines.
   */
  template<typename T, typename Lock>
  void ResumeAllMatches(Out<SuspendedRoutineQueue<T>> suspendedRoutines,
      const T& key, Lock& lock) {
    auto resumedRoutines = SuspendedRoutineQueue<T>();
    auto i = suspendedRoutines->begin();
    while(i != suspendedRoutines->end()) {
      if(i->m_key == key) {
        auto resumedRoutine = &*i;
        i = suspendedRoutines->erase(i);
        resumedRoutines.push_back(*resumedRoutine);
      } else {
        ++i;
      }
    }
    if(resumedRoutines.empty()) {
      return;
    }
    auto release = Threading::Release(lock);
    auto isInsideRoutine = Routine::IsInsideRoutine();
    Routine::IsInsideRoutine() = false;
    Resume(Store(resumedRoutines));
    Routine::IsInsideRoutine() = isInsideRoutine;
  }

  template<typename T>
  SuspendedRoutineNode<T>::SuspendedRoutineNode()
    : m_routine(&GetCurrentRoutine()) {}
}

#endif
