module;
#include "Prelude.hpp"

export module Beam:AuthenticationException;

export namespace Beam {

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

