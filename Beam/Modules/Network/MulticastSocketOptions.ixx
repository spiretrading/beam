module;
#include "Prelude.hpp"

export module Beam:MulticastSocketOptions;

import :UdpSocketOptions;

export namespace Beam {

  /** Stores the various options that can be applied to a MulticastSocket. */
  struct MulticastSocketOptions : UdpSocketOptions {
    using UdpSocketOptions::UdpSocketOptions;
  };
}

