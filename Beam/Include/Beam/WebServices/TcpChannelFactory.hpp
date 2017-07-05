#ifndef BEAM_WEBSERVICES_TCPCHANNELFACTORY_HPP
#define BEAM_WEBSERVICES_TCPCHANNELFACTORY_HPP
#include <memory>
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class TcpSocketChannelFactory
      \brief Implements a Channel factory using either a TcpSocketChannel or
             SecureSocketChannel depending on the protocol.
   */
  class TcpSocketChannelFactory {
    public:

      //! Constructs a TcpSocketChannelFactory.
      /*!
        \param socketThreadPool The SocketThreadPool all constructed Channels
               should use.
      */
      TcpSocketChannelFactory(
        RefType<Network::SocketThreadPool> socketThreadPool);

      //! Returns a new Channel.
      /*!
        \param uri The URI that the Channel should connect to.
      */
      std::unique_ptr<IO::VirtualChannel> operator ()(const Uri& uri) const;

    private:
      Network::SocketThreadPool* m_socketThreadPool;
  };

  inline TcpSocketChannelFactory::TcpSocketChannelFactory(
      RefType<Network::SocketThreadPool> socketThreadPool)
      : m_socketThreadPool{socketThreadPool.Get()} {}

  inline std::unique_ptr<IO::VirtualChannel>
      TcpSocketChannelFactory::operator ()(const Uri& url) const {
    Network::IpAddress address{url.GetHostname(), url.GetPort()};
    if(url.GetScheme() == "https" || url.GetScheme() == "wss") {
      auto baseSocket = std::make_unique<Network::SecureSocketChannel>(
        std::move(address), Ref(*m_socketThreadPool));
      return IO::MakeVirtualChannel(std::move(baseSocket));
    } else {
      auto baseSocket = std::make_unique<Network::TcpSocketChannel>(
        std::move(address), Ref(*m_socketThreadPool));
      return IO::MakeVirtualChannel(std::move(baseSocket));
    }
  }
}
}

#endif
