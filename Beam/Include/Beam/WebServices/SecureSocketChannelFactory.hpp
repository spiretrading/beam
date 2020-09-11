#ifndef BEAM_WEB_SERVICES_SECURE_SOCKET_CHANNEL_FACTORY_HPP
#define BEAM_WEB_SERVICES_SECURE_SOCKET_CHANNEL_FACTORY_HPP
#include <memory>
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Implements a Channel factory for a SecureSocketChannel. */
  class SecureSocketChannelFactory {
    public:

      /**
       * Returns a new SecureSocketChannel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<Network::SecureSocketChannel> operator ()(
        const Uri& uri) const;
  };

  inline std::unique_ptr<Network::SecureSocketChannel>
      SecureSocketChannelFactory::operator ()(const Uri& url) const {
    auto address = Network::IpAddress(url.GetHostname(), url.GetPort());
    return std::make_unique<Network::SecureSocketChannel>(std::move(address));
  }
}

#endif
