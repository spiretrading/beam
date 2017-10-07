#ifndef BEAM_WEBSERVICES_SOCKETCHANNELFACTORY_HPP
#define BEAM_WEBSERVICES_SOCKETCHANNELFACTORY_HPP
#include <memory>
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class SocketChannelFactory
      \brief Implements a Channel factory for a TcpSocketChannel.
   */
  class SocketChannelFactory {
    public:

      //! Constructs a SocketChannelFactory.
      /*!
        \param socketThreadPool The SocketThreadPool all constructed Channels
               should use.
      */
      SocketChannelFactory(RefType<Network::SocketThreadPool> socketThreadPool);

      //! Returns a new TcpSocketChannel.
      /*!
        \param uri The URI that the Channel should connect to.
      */
      std::unique_ptr<Network::TcpSocketChannel> operator ()(
        const Uri& uri) const;

    private:
      Network::SocketThreadPool* m_socketThreadPool;
  };

  inline SocketChannelFactory::SocketChannelFactory(
      RefType<Network::SocketThreadPool> socketThreadPool)
      : m_socketThreadPool{socketThreadPool.Get()} {}

  inline std::unique_ptr<Network::TcpSocketChannel>
      SocketChannelFactory::operator ()(const Uri& url) const {
    Network::IpAddress address{url.GetHostname(), url.GetPort()};
    return std::make_unique<Network::TcpSocketChannel>(std::move(address),
      Ref(*m_socketThreadPool));
  }
}
}

#endif
