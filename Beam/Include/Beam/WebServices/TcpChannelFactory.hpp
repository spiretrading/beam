#ifndef BEAM_WEB_SERVICES_TCP_CHANNEL_FACTORY_HPP
#define BEAM_WEB_SERVICES_TCP_CHANNEL_FACTORY_HPP
#include <boost/optional/optional.hpp>
#include <memory>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {

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
      TcpSocketChannelFactory(IpAddress interface) noexcept;

      /**
       * Returns a new Channel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<Channel> operator ()(const Uri& uri) const;

    private:
      boost::optional<IpAddress> m_interface;
  };

  inline TcpSocketChannelFactory::TcpSocketChannelFactory(
    IpAddress interface) noexcept
    : m_interface(std::move(interface)) {}

  inline std::unique_ptr<Channel> TcpSocketChannelFactory::operator ()(
      const Uri& url) const {
    auto address = IpAddress(url.get_hostname(), url.get_port());
    if(url.get_scheme() == "https" || url.get_scheme() == "wss") {
      auto base_socket = [&] {
        if(m_interface) {
          return std::make_unique<SecureSocketChannel>(
            std::move(address), *m_interface);
        }
        return std::make_unique<SecureSocketChannel>(std::move(address));
      }();
      return std::make_unique<Channel>(std::move(base_socket));
    } else {
      auto base_socket = [&] {
        if(m_interface) {
          return std::make_unique<TcpSocketChannel>(
            std::move(address), *m_interface);
        }
        return std::make_unique<TcpSocketChannel>(std::move(address));
      }();
      return std::make_unique<Channel>(std::move(base_socket));
    }
  }
}

#endif
