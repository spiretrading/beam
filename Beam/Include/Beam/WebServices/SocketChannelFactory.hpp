#ifndef BEAM_WEB_SERVICES_SOCKET_CHANNEL_FACTORY_HPP
#define BEAM_WEB_SERVICES_SOCKET_CHANNEL_FACTORY_HPP
#include <memory>
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Implements a Channel factory for a TcpSocketChannel. */
  class SocketChannelFactory {
    public:

      /**
       * Returns a new TcpSocketChannel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<Network::TcpSocketChannel> operator ()(
        const Uri& uri) const;
  };

  inline std::unique_ptr<Network::TcpSocketChannel>
      SocketChannelFactory::operator ()(const Uri& url) const {
    auto address = Network::IpAddress(url.GetHostname(), url.GetPort());
    return std::make_unique<Network::TcpSocketChannel>(std::move(address));
  }
}

#endif
