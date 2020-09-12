#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#define BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#include <boost/intrusive/list.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Routines.hpp"

namespace Beam::Routines {

  /** Stores an intrusive node to the current Routine. */
  struct SuspendedRoutineNode : public boost::intrusive::list_base_hook<> {

    /** The suspended routine. */
    Routines::Routine* m_routine;

    /** Constructs a SuspendedRoutineNode representing the current Routine. */
    SuspendedRoutineNode();
  };

  /** An intrusive linked list of suspended Routines. */
  using SuspendedRoutineQueue = boost::intrusive::list<SuspendedRoutineNode>;

  /**
   * Suspends the currently running Routine.
   * @param suspendedRoutines Stores the Routine being suspended.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename... Lock>
  void Suspend(Out<SuspendedRoutineQueue> suspendedRoutines, Lock&... lock);

  /**
   * Resumes the first Routine found in a queue of suspended Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   */
  template<typename... Lock>
  void ResumeFront(Out<SuspendedRoutineQueue> suspendedRoutines);

  /**
   * Resumes all Routines found in a queue of suspended Routines.
   * @param suspendedRoutines Stores the Routines being suspended.
   */
  template<typename... Lock>
  void Resume(Out<SuspendedRoutineQueue> suspendedRoutines);
}

#endif
