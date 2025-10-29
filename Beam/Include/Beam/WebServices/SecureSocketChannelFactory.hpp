#ifndef BEAM_WEB_SERVICES_SECURE_SOCKET_CHANNEL_FACTORY_HPP
#define BEAM_WEB_SERVICES_SECURE_SOCKET_CHANNEL_FACTORY_HPP
#include <memory>
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {

  /** Implements a Channel factory for a SecureSocketChannel. */
  class SecureSocketChannelFactory {
    public:

      /**
       * Returns a new SecureSocketChannel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<SecureSocketChannel> operator ()(const Uri& uri) const;
  };

  inline std::unique_ptr<SecureSocketChannel>
      SecureSocketChannelFactory::operator ()(const Uri& url) const {
    auto address = IpAddress(url.get_hostname(), url.get_port());
    return std::make_unique<SecureSocketChannel>(std::move(address));
  }
}

#endif
