#ifndef BEAM_NOT_CONNECTED_EXCEPTION_HPP
#define BEAM_NOT_CONNECTED_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam::IO {

  /** Indicates an operation failed due to not being connected. */
  class NotConnectedException : public IOException {
    public:

      /** Constructs a NotConnectedException. */
      NotConnectedException();

      /**
       * Constructs a NotConnectedException.
       * @param message A message describing the error.
       */
      NotConnectedException(const std::string& message);
  };

  inline NotConnectedException::NotConnectedException()
    : IOException("Not connected.") {}

  inline NotConnectedException::NotConnectedException(
    const std::string& message)
    : IOException(message) {}
}

#endif
