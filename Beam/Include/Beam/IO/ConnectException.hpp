#ifndef BEAM_CONNECT_EXCEPTION_HPP
#define BEAM_CONNECT_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam {

  /** Signals that a connect operation failed. */
  class ConnectException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs a ConnectException. */
      ConnectException();
  };

  inline ConnectException::ConnectException()
    : ConnectException("Unable to connect.") {}
}

#endif
