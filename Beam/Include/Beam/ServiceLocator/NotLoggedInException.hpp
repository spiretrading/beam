#ifndef BEAM_NOT_LOGGED_IN_EXCEPTION_HPP
#define BEAM_NOT_LOGGED_IN_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** An service was requested without being logged in. */
  class NotLoggedInException : public IO::IOException {
    public:
      using IO::IOException::IOException;

      /** Constructs a NotLoggedInException. */
      NotLoggedInException();
  };

  inline NotLoggedInException::NotLoggedInException()
    : IO::IOException("Client is not logged in.") {}
}

#endif
