#ifndef BEAM_MULTICAST_SOCKET_OPTIONS_HPP
#define BEAM_MULTICAST_SOCKET_OPTIONS_HPP
#include "Beam/Network/UdpSocketOptions.hpp"

namespace Beam::Network {

  /** Stores the various options that can be applied to a MulticastSocket. */
  struct MulticastSocketOptions : UdpSocketOptions {
    using UdpSocketOptions::UdpSocketOptions;
  };
}

#endif
