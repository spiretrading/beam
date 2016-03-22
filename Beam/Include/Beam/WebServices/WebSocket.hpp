#ifndef BEAM_WEBSOCKET_HPP
#define BEAM_WEBSOCKET_HPP
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocket
      \brief Implements a WebSocket connection to a server.
      \tparam ChannelType The type of Channel used to connect to the server.
   */
  template<typename ChannelType>
  class WebSocket : private boost::noncopyable {
    public:

      //! The type of Channel used to connect to the server.
      using Channel = GetTryDereferenceType<ChannelType>;

      //! Constructs a WebSocket.
      /*!
        \param uri The URL to connect to.
        \param channel Initializes the Channel used to connect to the server.
      */
      template<typename ChannelForward>
      WebSocket(Uri uri, ChannelForward&& channel);

      //! Constructs a WebSocket.
      /*!
        \param uri The URL to connect to.
        \param protocols The list of protocols used to indicate sub-protocols.
        \param channel Initializes the Channel used to connect to the server.
      */
      template<typename ChannelForward>
      WebSocket(Uri uri, std::vector<std::string> protocols,
        ChannelForward&& channel);

      ~WebSocket();

      void Open();

      void Close();

    private:
      Uri m_uri;
      std::vector<std::string> m_protocols;
      int m_version;
      GetOptionalLocalPtr<ChannelType> m_channel;
  };

  template<typename ChannelType>
  template<typename ChannelForward>
  WebSocket<ChannelType>::WebSocket(Uri uri, ChannelForward&& channel)
      : WebSocket{std::move(uri), std::vector<std::string>(),
          std::forward<ChannelForward>(channel)} {}

  template<typename ChannelType>
  template<typename ChannelForward>
  WebSocket<ChannelType>::WebSocket(Uri uri, std::vector<std::string> protocols,
      ChannelForward&& channel)
      : m_uri{std::move(uri)},
        m_protocols{std::move(protocols)},
        m_version{0},
        m_channel{std::move(channel)} {}

  template <typename ChannelType>
  WebSocket<ChannelType>::~WebSocket() {
    Close();
  }

  template<typename ChannelType>
  void WebSocket<ChannelType>::Open() {
    HttpRequest request{HttpMethod::GET, m_uri.GetPath(),
      HttpVersion::Version1_1()};
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
    if(m_version != 0) {
      request.Add(HttpHeader{"Sec-WebSocket-Version",
        std::to_string(m_version)});
    }
    typename Channel::Writer::Buffer sendBuffer;
    IO::BufferOutputStream<typename Channel::Writer::Buffer> sendStream{
      Ref(sendBuffer)};
    sendStream << request;
    sendStream.flush();
    m_channel->Open();
    m_channel->GetWriter().Write(sendBuffer);
    typename Channel::Reader::Buffer receiveBuffer;
    m_channel->GetReader().Read(Store(receiveBuffer));
    std::cout << receiveBuffer << std::endl;
  }

  template<typename ChannelType>
  void WebSocket<ChannelType>::Close() {
    m_channel->Close();
  }
}
}

#endif
