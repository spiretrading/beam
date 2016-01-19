#ifndef BEAM_ROUTINES_HPP
#define BEAM_ROUTINES_HPP

namespace Beam {
namespace Routines {
  template<typename T> class Async;
  class BaseAsync;
  class BaseEval;
  template<typename T> class Eval;
  class ExternalRoutine;
  template<typename F> class FunctionRoutine;
  class Routine;
  class RoutineException;
  class RoutineHandler;
  class ScheduledRoutine;
namespace Details {
  class Scheduler;
}
}
}

#endif
