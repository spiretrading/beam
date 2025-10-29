#ifndef BEAM_ROUTINE_EXCEPTION_HPP
#define BEAM_ROUTINE_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Used to indicate an exception occurred regarding a Routine. */
  class RoutineException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
