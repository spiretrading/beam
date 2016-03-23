#ifndef BEAM_WEBSOCKET_HPP
#define BEAM_WEBSOCKET_HPP
#include <functional>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocket
      \brief Implements a WebSocket connection to a server.
      \tparam ChannelType The type Channel used to connect to the server.
   */
  template<typename ChannelType>
  class WebSocket : private boost::noncopyable {
    public:

      //! The type of Channel used to connect to the server.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! The factory used to build Channels.
      /*!
        \param uri The URI to connect to.
        \return A Channel able to connect to the specified <i>uri</i>.
      */
      using ChannelBuilder =
        std::function<std::unique_ptr<Channel> (const Uri& uri)>;

      //! Constructs a WebSocket.
      /*!
        \param uri The URL to connect to.
        \param channelBuilder Builds the Channel used to connect to the server.
      */
      WebSocket(Uri uri, ChannelBuilder channelBuilder);

      //! Constructs a WebSocket.
      /*!
        \param uri The URI to connect to.
        \param protocols The list of protocols used to indicate sub-protocols.
        \param channelBuilder Builds the Channel used to connect to the server.
      */
      WebSocket(Uri uri, std::vector<std::string> protocols,
        ChannelBuilder channelBuilder);

      ~WebSocket();

      void Open();

      void Close();

    private:
      Uri m_uri;
      std::vector<std::string> m_protocols;
      std::string m_version;
      ChannelBuilder m_channelBuilder;
      HttpResponseParser m_parser;
      std::unique_ptr<ChannelType> m_channel;
      IO::OpenState m_openState;

      void Shutdown();
  };

  template<typename ChannelType>
  WebSocket<ChannelType>::WebSocket(Uri uri, ChannelBuilder channelBuilder)
      : WebSocket{std::move(uri), std::vector<std::string>(),
          std::move(channelBuilder)} {}

  template<typename ChannelType>
  WebSocket<ChannelType>::WebSocket(Uri uri, std::vector<std::string> protocols,
      ChannelBuilder channelBuilder)
      : m_uri{std::move(uri)},
        m_protocols{std::move(protocols)},
        m_version{"13"},
        m_channelBuilder{std::move(channelBuilder)} {
    if(m_uri.GetPort() == 0) {
      if(m_uri.GetScheme() == "http" || m_uri.GetScheme() == "ws") {
        m_uri.SetPort(80);
      } else if(m_uri.GetScheme() == "https" || m_uri.GetScheme() == "wss") {
        m_uri.SetPort(443);
      }
    }
  }

  template <typename ChannelType>
  WebSocket<ChannelType>::~WebSocket() {
    Close();
  }

  template<typename ChannelType>
  void WebSocket<ChannelType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      HttpRequest request{HttpVersion::Version1_1(), HttpMethod::GET, m_uri};
      request.Add(HttpHeader{"Upgrade", "websocket"});
      request.Add(HttpHeader{"Connection", "Upgrade"});
      auto key = "x3JJHMbDL1EzLkh9GBhXDw==";
      request.Add(HttpHeader{"Sec-WebSocket-Key", key});
      if(!m_protocols.empty()) {
        std::string protocols;
        auto isFirst = true;
        for(auto& protocol : m_protocols) {
          if(isFirst) {
            isFirst = false;
          } else {
            protocols += ", ";
          }
          protocols += protocol;
        }
        request.Add(HttpHeader{"Sec-WebSocket-Protocol", protocols});
      }
      if(!m_version.empty()) {
        request.Add(HttpHeader{"Sec-WebSocket-Version", m_version});
      }
      typename Channel::Writer::Buffer sendBuffer;
      IO::BufferOutputStream<typename Channel::Writer::Buffer> sendStream{
        Ref(sendBuffer)};
      sendStream << request;
      sendStream.flush();
      m_channel = m_channelBuilder(m_uri);
      m_channel->GetConnection().Open();
      m_channel->GetWriter().Write(sendBuffer);
      typename Channel::Reader::Buffer receiveBuffer;
      while(true) {
        m_channel->GetReader().Read(Store(receiveBuffer));
        m_parser.Feed(receiveBuffer.GetData(), receiveBuffer.GetSize());
        auto response = m_parser.GetNextResponse();
        receiveBuffer.Reset();
        if(response.is_initialized()) {
          if(response->GetStatusCode() == HttpStatusCode::SWITCHING_PROTOCOLS) {
            break;
          } else {
            BOOST_THROW_EXCEPTION(IO::ConnectException{"Invalid status code: " +
              GetReasonPhrase(response->GetStatusCode())});
          }
        }
      }
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ChannelType>
  void WebSocket<ChannelType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ChannelType>
  void WebSocket<ChannelType>::Shutdown() {
    m_channel->GetConnection().Close();
    m_openState.SetClosed();
  }
}
}

#endif
