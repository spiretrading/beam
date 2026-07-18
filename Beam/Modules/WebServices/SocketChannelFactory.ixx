module;
#include "Prelude.hpp"

export module Beam:SocketChannelFactory;

import :Uri;

export namespace Beam {

  /** Implements a Channel factory for a TcpSocketChannel. */
  class SocketChannelFactory {
    public:

      /**
       * Returns a new TcpSocketChannel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<TcpSocketChannel> operator ()(const Uri& uri) const;
  };

  inline std::unique_ptr<TcpSocketChannel>
      SocketChannelFactory::operator ()(const Uri& uri) const {
    auto address = IpAddress(uri.get_hostname(), uri.get_port());
    return std::make_unique<TcpSocketChannel>(std::move(address));
  }
}

