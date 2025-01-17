#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#define BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#include <boost/intrusive/list.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Routines.hpp"

namespace Beam::Routines {

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
  void Suspend(Out<SuspendedRoutineQueue<>> suspendedRoutines, Lock&... lock);

  /**
   * Suspends the currently running Routine.
   * @param suspendedRoutines Stores the Routine being suspended.
   * @param key The key to associated with the current routine.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename T, typename... Lock>
  void Suspend(
    Out<SuspendedRoutineQueue<T>> suspendedRoutines, T key, Lock&... lock);

  /**
   * Resumes the first Routine in a queue of suspended Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   */
  template<typename T>
  void ResumeFront(Out<SuspendedRoutineQueue<T>> suspendedRoutines);

  /**
   * Resumes the first Routine found that matches a key in a queue of suspended
   * Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   * @param key The key to find.
   */
  template<typename T>
  void ResumeFirstMatch(
    Out<SuspendedRoutineQueue<T>> suspendedRoutines, const T& key);

  /**
   * Resumes all Routines found that match a key in a queue of suspended
   * Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   * @param key The key to find.
   */
  template<typename T>
  void ResumeAllMatches(
    Out<SuspendedRoutineQueue<T>> suspendedRoutines, const T& key);

  /**
   * Resumes all Routines found in a queue of suspended Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   */
  template<typename T>
  void Resume(Out<SuspendedRoutineQueue<T>> suspendedRoutines);
}

#endif
