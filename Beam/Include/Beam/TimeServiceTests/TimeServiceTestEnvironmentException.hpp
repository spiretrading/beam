#ifndef BEAM_TIMESERVICETESTENVIRONMENTEXCEPTION_HPP
#define BEAM_TIMESERVICETESTENVIRONMENTEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/TimeServiceTests/TimeServiceTests.hpp"

namespace Beam {
namespace TimeService {
namespace Tests {

  /*! \class TimeServiceTestEnvironmentException
      \brief Signals an invalid operation performed on a
             TimeServiceTestEnvironment.
   */
  class TimeServiceTestEnvironmentException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a TimeServiceTestEnvironmentException.
      TimeServiceTestEnvironmentException();

      //! Constructs a TimeServiceTestEnvironmentException.
      /*!
        \param message A message describing the error.
      */
      TimeServiceTestEnvironmentException(const std::string& message);
  };

  inline TimeServiceTestEnvironmentException::
      TimeServiceTestEnvironmentException()
      : TimeServiceTestEnvironmentException{"Invalid operation performed."} {}

  inline TimeServiceTestEnvironmentException::
      TimeServiceTestEnvironmentException(const std::string& message)
      : std::runtime_error{message} {}
}
}
}

#endif
