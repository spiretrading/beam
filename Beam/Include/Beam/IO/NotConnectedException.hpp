#ifndef BEAM_NOT_CONNECTED_EXCEPTION_HPP
#define BEAM_NOT_CONNECTED_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam::IO {

  /** Indicates an operation failed due to not being connected. */
  class NotConnectedException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs a NotConnectedException. */
      NotConnectedException();
  };

  inline NotConnectedException::NotConnectedException()
    : IOException("Not connected.") {}
}

#endif
