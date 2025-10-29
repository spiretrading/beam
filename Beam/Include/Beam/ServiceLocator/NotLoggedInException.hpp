#ifndef BEAM_NOT_LOGGED_IN_EXCEPTION_HPP
#define BEAM_NOT_LOGGED_IN_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam {

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

#endif
