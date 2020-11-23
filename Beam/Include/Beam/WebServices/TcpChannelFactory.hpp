#ifndef BEAM_WEB_SERVICES_TCP_CHANNEL_FACTORY_HPP
#define BEAM_WEB_SERVICES_TCP_CHANNEL_FACTORY_HPP
#include <boost/optional/optional.hpp>
#include <memory>
#include "Beam/IO/ChannelBox.hpp"
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /**
   * Implements a Channel factory using either a TcpSocketChannel or
   * SecureSocketChannel depending on the protocol.
   */
  class TcpSocketChannelFactory {
    public:

      /** Constructs a TcpSocketChannelFactory. */
      TcpSocketChannelFactory() = default;

      /**
       * Constructs a TcpSocketChannelFactory.
       * @param interface The interface to bind to.
       */
      TcpSocketChannelFactory(Network::IpAddress interface);

      /**
       * Returns a new Channel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<IO::ChannelBox> operator ()(const Uri& uri) const;

    private:
      boost::optional<Network::IpAddress> m_interface;
  };

  inline TcpSocketChannelFactory::TcpSocketChannelFactory(
    Network::IpAddress interface)
    : m_interface(std::move(interface)) {}

  inline std::unique_ptr<IO::ChannelBox> TcpSocketChannelFactory::operator ()(
      const Uri& url) const {
    auto address = Network::IpAddress(url.GetHostname(), url.GetPort());
    if(url.GetScheme() == "https" || url.GetScheme() == "wss") {
      auto baseSocket = [&] {
        if(m_interface) {
          return std::make_unique<Network::SecureSocketChannel>(
            std::move(address), *m_interface);
        }
        return std::make_unique<Network::SecureSocketChannel>(
          std::move(address));
      }();
      return std::make_unique<IO::ChannelBox>(std::move(baseSocket));
    } else {
      auto baseSocket = [&] {
        if(m_interface) {
          return std::make_unique<Network::TcpSocketChannel>(std::move(address),
            *m_interface);
        }
        return std::make_unique<Network::TcpSocketChannel>(std::move(address));
      }();
      return std::make_unique<IO::ChannelBox>(std::move(baseSocket));
    }
  }
}

#endif
