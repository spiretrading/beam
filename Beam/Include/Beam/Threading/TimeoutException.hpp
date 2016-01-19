#ifndef BEAM_TIMEOUTEXCEPTION_HPP
#define BEAM_TIMEOUTEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class TimeoutException
      \brief Signals that an operation timed out.
   */
  class TimeoutException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs a TimeoutException.
      TimeoutException();

      //! Constructs a TimeoutException.
      /*!
        \param message A message describing the error.
      */
      TimeoutException(const std::string& message);
  };

  inline TimeoutException::TimeoutException()
    : std::runtime_error("Operation timed out.") {}

  inline TimeoutException::TimeoutException(const std::string& message)
    : std::runtime_error(message) {}
}
}

#endif
