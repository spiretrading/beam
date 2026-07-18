module;
#include "Prelude.hpp"

export module Beam:RoutineException;

export namespace Beam {

  /** Used to indicate an exception occurred regarding a Routine. */
  class RoutineException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

