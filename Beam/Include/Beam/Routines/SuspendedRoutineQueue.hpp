#ifndef BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#define BEAM_SUSPENDED_ROUTINE_QUEUE_HPP
#include <boost/intrusive/list.hpp>
#include "Beam/Pointers/Out.hpp"

namespace Beam {
  class Routine;

  /** Stores an intrusive node to the current Routine. */
  struct SuspendedRoutineNode : public boost::intrusive::list_base_hook<> {

    /** The suspended routine. */
    Routine* m_routine;

    /** Constructs a SuspendedRoutineNode representing the current Routine. */
    SuspendedRoutineNode() noexcept;
  };

  /** An intrusive linked list of suspended Routines. */
  using SuspendedRoutineQueue = boost::intrusive::list<SuspendedRoutineNode>;

  /**
   * Suspends the currently running Routine.
   * @param routines Stores the Routine being suspended.
   * @param lock The lock to release while the Routine is suspended.
   */
  template<typename... Lock>
  void suspend(Out<SuspendedRoutineQueue> routines, Lock&... lock);

  /**
   * Resumes the first Routine found in a queue of suspended Routines.
   * @param routines Stores the Routines being suspended.
   */
  template<typename... Lock>
  void resume_front(Out<SuspendedRoutineQueue> routines);

  /**
   * Resumes all Routines found in a queue of suspended Routines.
   * @param routines Stores the Routines being suspended.
   */
  template<typename... Lock>
  void resume(Out<SuspendedRoutineQueue> routines);
}

#endif
