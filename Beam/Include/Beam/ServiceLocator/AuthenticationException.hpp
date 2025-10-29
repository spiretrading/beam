#ifndef BEAM_AUTHENTICATION_EXCEPTION_HPP
#define BEAM_AUTHENTICATION_EXCEPTION_HPP
#include "Beam/IO/ConnectException.hpp"

namespace Beam {

  /** Signals an authentication error during a connect operation. */
  class AuthenticationException : public ConnectException {
    public:
      using ConnectException::ConnectException;

      /** Constructs an AuthenticationException. */
      AuthenticationException();
  };

  inline AuthenticationException::AuthenticationException()
    : AuthenticationException("Unable to authenticate connection.") {}
}

#endif
