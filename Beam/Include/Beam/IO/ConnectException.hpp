#ifndef BEAM_CONNECT_EXCEPTION_HPP
#define BEAM_CONNECT_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam::IO {

  /** Signals that a connect operation failed. */
  class ConnectException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs a ConnectException. */
      ConnectException();
  };

  inline ConnectException::ConnectException()
    : IOException("Unable to connect.") {}
}

#endif
