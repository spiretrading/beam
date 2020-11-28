#ifndef BEAM_AUTHENTICATION_EXCEPTION_HPP
#define BEAM_AUTHENTICATION_EXCEPTION_HPP
#include "Beam/IO/ConnectException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Signals an authentication error during a connect operation. */
  class AuthenticationException : public IO::ConnectException {
    public:
      using IO::ConnectException::ConnectException;

      /** Constructs an AuthenticationException. */
      AuthenticationException();
  };

  inline AuthenticationException::AuthenticationException()
    : IO::ConnectException("Unable to authenticate connection.") {}
}

#endif
