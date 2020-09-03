#ifndef BEAM_NETWORK_SECURE_SOCKET_OPTIONS_HPP
#define BEAM_NETWORK_SECURE_SOCKET_OPTIONS_HPP
#include "Beam/Network/TcpSocketOptions.hpp"

namespace Beam::Network {

  /**
   * Stores the various options that can be applied to a SecureSocketChannel.
   */
  struct SecureSocketOptions : TcpSocketOptions {
    using TcpSocketOptions::TcpSocketOptions;
  };
}

#endif
