module;
#include "Prelude.hpp"

export module Beam:NotLoggedInException;

export namespace Beam {

  /** An service was requested without being logged in. */
  class NotLoggedInException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs a NotLoggedInException. */
      NotLoggedInException();
  };

  inline NotLoggedInException::NotLoggedInException()
    : NotLoggedInException("Client is not logged in.") {}
}

