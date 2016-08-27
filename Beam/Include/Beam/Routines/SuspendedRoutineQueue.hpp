#ifndef BEAM_SUSPENDEDROUTINEQUEUE_HPP
#define BEAM_SUSPENDEDROUTINEQUEUE_HPP
#include <boost/intrusive/list.hpp>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/Routines.hpp"

namespace Beam {
namespace Routines {

  /*! \struct SuspendedRoutineNode
      \brief Stores an intrusive node to the current Routine.
   */
  struct SuspendedRoutineNode : public boost::intrusive::list_base_hook<> {

    //! The suspended routine.
    Routines::Routine* m_routine;

    SuspendedRoutineNode();
  };

  //! An intrusive linked list of suspended Routines.
  using SuspendedRoutineQueue = boost::intrusive::list<SuspendedRoutineNode>;

  inline SuspendedRoutineNode::SuspendedRoutineNode()
      : m_routine{&GetCurrentRoutine()} {}
}
}

#endif
