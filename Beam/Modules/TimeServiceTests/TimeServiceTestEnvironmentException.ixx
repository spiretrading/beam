module;
#include "Prelude.hpp"

export module Beam:TimeServiceTestEnvironmentException;

export namespace Beam::Tests {

  /** Signals an invalid operation performed on a TimeServiceTestEnvironment. */
  class TimeServiceTestEnvironmentException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a TimeServiceTestEnvironmentException. */
      TimeServiceTestEnvironmentException();
  };

  inline TimeServiceTestEnvironmentException::
    TimeServiceTestEnvironmentException()
    : TimeServiceTestEnvironmentException("Invalid operation performed.") {}
}

