#ifndef BEAM_HTTPCLIENT_HPP
#define BEAM_HTTPCLIENT_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/WebServices.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpClient
      \brief A client that can submit HTTP requests to a server.
      \tparam ChannelType The type Channel used to connect to the server.
   */
  template<typename ChannelType>
  class HttpClient : private boost::noncopyable {
    public:

      //! The type of Channel used to connect to the server.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! The factory used to build Channels.
      /*!
        \param uri The URI to connect to.
        \return A Channel able to connect to the specified <i>uri</i>.
      */
      using ChannelBuilder = std::function<ChannelType (const Uri& uri)>;

      //! Constructs an HttpClient.
      /*!
        \param channelBuilder Builds the Channel used to connect to the server.
      */
      HttpClient(ChannelBuilder channelBuilder);

      //! Sends a request.
      /*!
        \param request The HttpRequest to send.
        \return The response to the specified <i>request</i>.
      */
      HttpResponse Send(const HttpRequest& request);

    private:
      struct ChannelEntry {
        Network::IpAddress m_endPoint;
        ChannelType m_channel;

        ChannelEntry(Network::IpAddress endPoint, ChannelType channel);
      };
      ChannelBuilder m_channelBuilder;
      boost::optional<ChannelEntry> m_channel;
  };

  template<typename ChannelType>
  HttpClient<ChannelType>::ChannelEntry::ChannelEntry(
      Network::IpAddress endPoint, ChannelType channel)
      : m_endPoint{std::move(endPoint)},
        m_channel{std::move(channel)} {}

  template<typename ChannelType>
  HttpClient<ChannelType>::HttpClient(ChannelBuilder channelBuilder)
      : m_channelBuilder{std::move(channelBuilder)} {}

  template<typename ChannelType>
  HttpResponse HttpClient<ChannelType>::Send(const HttpRequest& request) {
    auto endPoint = Network::IpAddress{
      request.GetUri().GetHostname(), request.GetUri().GetPort()};
    auto isNewChannel = false;
    if(!m_channel.is_initialized() || m_channel->m_endPoint != endPoint) {
      m_channel.reset();
      m_channel.emplace(endPoint, m_channelBuilder(request.GetUri()));
      m_channel->m_channel->GetConnection().Open();
      isNewChannel = true;
    }
    {
      typename Channel::Writer::Buffer writeBuffer;
      request.Encode(Store(writeBuffer));
      try {
        m_channel->m_channel->GetWriter().Write(writeBuffer);
      } catch(const std::exception&) {
        if(isNewChannel) {
          throw;
        }
        m_channel.reset();
        Send(request);
      }
    }
    HttpResponseParser parser;
    auto response = parser.GetNextResponse();
    while(!response.is_initialized()) {
      typename Channel::Reader::Buffer readBuffer;
      m_channel->m_channel->GetReader().Read(Store(readBuffer));
      parser.Feed(readBuffer.GetData(), readBuffer.GetSize());
      response = parser.GetNextResponse();
    }
    auto connectionHeader = response->GetHeader("Connection");
    if(!connectionHeader.is_initialized()) {
      if(response->GetVersion() == HttpVersion::Version1_0()) {
        m_channel->m_channel->GetConnection().Close();
        m_channel.reset();
      }
    } else {
      if(!boost::iequals(*connectionHeader, "keep-alive")) {
        m_channel->m_channel->GetConnection().Close();
        m_channel.reset();
      }
    }
    return *response;
  }
}
}

#endif
