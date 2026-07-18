module;
#include "Prelude.hpp"

export module Beam:TimeoutException;

export namespace Beam {

  /** Signals that an operation timed out. */
  class TimeoutException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a TimeoutException. */
      TimeoutException();
  };

  inline TimeoutException::TimeoutException()
    : TimeoutException("Operation timed out.") {}
}

