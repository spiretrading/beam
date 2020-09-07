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

      /** Constructs a TimeServiceTestEnvironmentException. */
      TimeServiceTestEnvironmentException();

      /**
       * Constructs a TimeServiceTestEnvironmentException.
       * @param message A message describing the error.
       */
      TimeServiceTestEnvironmentException(const std::string& message);
  };

  inline TimeServiceTestEnvironmentException::
    TimeServiceTestEnvironmentException()
    : TimeServiceTestEnvironmentException("Invalid operation performed.") {}

  inline TimeServiceTestEnvironmentException::
    TimeServiceTestEnvironmentException(const std::string& message)
    : std::runtime_error(message) {}
}

#endif
