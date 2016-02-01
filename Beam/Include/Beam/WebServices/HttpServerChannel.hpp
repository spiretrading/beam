#ifndef AVALON_HTTPSERVERCHANNEL_HPP
#define AVALON_HTTPSERVERCHANNEL_HPP
#include "Avalon/IO/Channel.hpp"
#include "Avalon/WebServices/HttpProtocol.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpServerChannel
      \brief A Channel used by the HttpServer.
   */
  class HttpServerChannel : public IO::Channel {
    public:

      //! Constructs an HttpServerChannel.
      /*!
        \param channel The underlying Channel.
      */
      HttpServerChannel(IO::Channel* channel);

      ~HttpServerChannel();

      //! Returns the HttpProtocol.
      HttpProtocol& GetProtocol();

      virtual IO::Connection& GetConnection();

      virtual IO::Reader& GetReader();

      virtual IO::Writer& GetWriter();

    private:
      boost::scoped_ptr<IO::Channel> m_channel;
      HttpProtocol m_protocol;
  };
}
}

#endif // AVALON_HTTPSERVERCHANNEL_HPP
