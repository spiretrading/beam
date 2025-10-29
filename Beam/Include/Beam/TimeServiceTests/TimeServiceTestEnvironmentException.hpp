#ifndef BEAM_TIME_SERVICE_TEST_ENVIRONMENT_EXCEPTION_HPP
#define BEAM_TIME_SERVICE_TEST_ENVIRONMENT_EXCEPTION_HPP
#include <stdexcept>

namespace Beam::Tests {

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

#endif
