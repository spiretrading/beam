module;
#include "Prelude.hpp"

export module Beam:SecureSocketOptions;

export namespace Beam {

  /**
   * Stores the various options that can be applied to a SecureSocketChannel.
   */
  struct SecureSocketOptions : TcpSocketOptions {
    using TcpSocketOptions::TcpSocketOptions;
  };
}

