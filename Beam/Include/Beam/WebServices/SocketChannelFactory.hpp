#ifndef BEAM_WEB_SERVICES_SOCKET_CHANNEL_FACTORY_HPP
#define BEAM_WEB_SERVICES_SOCKET_CHANNEL_FACTORY_HPP
#include <memory>
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {

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

#endif
