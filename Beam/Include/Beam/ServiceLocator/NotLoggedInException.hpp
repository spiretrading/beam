#ifndef BEAM_NOT_LOGGED_IN_EXCEPTION_HPP
#define BEAM_NOT_LOGGED_IN_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** An service was requested without being logged in. */
  class NotLoggedInException : public IO::IOException {
    public:

      /** Constructs a NotLoggedInException. */
      NotLoggedInException();

      /**
       * Constructs a NotLoggedInException.
       * @param message A message describing the error.
       */
      explicit NotLoggedInException(const std::string& message);
  };

  inline NotLoggedInException::NotLoggedInException()
    : IO::IOException("Client is not logged in.") {}

  inline NotLoggedInException::NotLoggedInException(const std::string& message)
    : IO::IOException(message) {}
}

#endif
