#ifndef BEAM_TIME_SERVICE_TEST_ENVIRONMENT_EXCEPTION_HPP
#define BEAM_TIME_SERVICE_TEST_ENVIRONMENT_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/TimeServiceTests/TimeServiceTests.hpp"

namespace Beam::TimeService::Tests {

  /** Signals an invalid operation performed on a TimeServiceTestEnvironment. */
  class TimeServiceTestEnvironmentException : public std::runtime_error,
      public boost::exception {
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
