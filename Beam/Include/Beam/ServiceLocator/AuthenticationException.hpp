#ifndef BEAM_AUTHENTICATION_EXCEPTION_HPP
#define BEAM_AUTHENTICATION_EXCEPTION_HPP
#include "Beam/IO/ConnectException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Signals an authentication error during a connect operation. */
  class AuthenticationException : public IO::ConnectException {
    public:

      /** Constructs an AuthenticationException. */
      AuthenticationException();

      /**
       * Constructs an AuthenticationException.
       * @param message A message describing the error.
       */
      explicit AuthenticationException(const std::string& message);
  };

  inline AuthenticationException::AuthenticationException()
    : ConnectException("Unable to authenticate connection.") {}

  inline AuthenticationException::AuthenticationException(
    const std::string& message)
    : ConnectException(message) {}
}

#endif
