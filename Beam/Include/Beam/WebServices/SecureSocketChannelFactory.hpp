#ifndef BEAM_WEBSERVICES_SECURESOCKETCHANNELFACTORY_HPP
#define BEAM_WEBSERVICES_SECURESOCKETCHANNELFACTORY_HPP
#include <memory>
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class SecureSocketChannelFactory
      \brief Implements a Channel factory for a SecureSocketChannel.
   */
  class SecureSocketChannelFactory {
    public:

      //! Constructs a SecureSocketChannelFactory.
      /*!
        \param socketThreadPool The SocketThreadPool all constructed Channels
               should use.
      */
      SecureSocketChannelFactory(
        Ref<Network::SocketThreadPool> socketThreadPool);

      //! Returns a new SecureSocketChannel.
      /*!
        \param uri The URI that the Channel should connect to.
      */
      std::unique_ptr<Network::SecureSocketChannel> operator ()(
        const Uri& uri) const;

    private:
      Network::SocketThreadPool* m_socketThreadPool;
  };

  inline SecureSocketChannelFactory::SecureSocketChannelFactory(
      Ref<Network::SocketThreadPool> socketThreadPool)
      : m_socketThreadPool{socketThreadPool.Get()} {}

  inline std::unique_ptr<Network::SecureSocketChannel>
      SecureSocketChannelFactory::operator ()(const Uri& url) const {
    Network::IpAddress address{url.GetHostname(), url.GetPort()};
    return std::make_unique<Network::SecureSocketChannel>(std::move(address),
      Ref(*m_socketThreadPool));
  }
}
}

#endif
