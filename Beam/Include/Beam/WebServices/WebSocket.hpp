#ifndef BEAM_WEBSOCKET_HPP
#define BEAM_WEBSOCKET_HPP
#include <ctime>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocketConfig
      \brief Contains the configuration needed to construct a WebSocket.
   */
  class WebSocketConfig {
    public:

      //! Constructs a WebSocketConfig with default values.
      WebSocketConfig();

      //! Sets the URI to connect to.
      WebSocketConfig& SetUri(Uri uri);

      //! Sets the websocket version.
      WebSocketConfig& SetVersion(std::string version);

      //! Sets the list of protocols.
      WebSocketConfig& SetProtocols(std::vector<std::string> protocols);

      //! Sets the list of extensions.
      WebSocketConfig& SetExtensions(std::vector<std::string> extensions);

    private:
      template<typename> friend class WebSocket;
      Uri m_uri;
      std::string m_version;
      std::vector<std::string> m_protocols;
      std::vector<std::string> m_extensions;
  };

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
        \param config The config used to initialize this socket.
        \param channelBuilder Builds the Channel used to connect to the server.
      */
      WebSocket(WebSocketConfig config, ChannelBuilder channelBuilder);

      ~WebSocket();

      //! Reads the next frame from the web socket.
      IO::SharedBuffer Read();

      //! Writes to the web socket.
      /*!
        \param buffer The buffer to write.
      */
      template<typename Buffer>
      void Write(const Buffer& buffer);

      void Open();

      void Close();

    private:
      Uri m_uri;
      std::vector<std::string> m_protocols;
      std::vector<std::string> m_extensions;
      std::string m_version;
      ChannelBuilder m_channelBuilder;
      HttpResponseParser m_parser;
      std::unique_ptr<ChannelType> m_channel;
      std::mt19937 m_randomEngine;
      typename Channel::Reader::Buffer m_frameBuffer;
      IO::OpenState m_openState;

      void Shutdown();
  };

  inline WebSocketConfig::WebSocketConfig()
      : m_version{"13"} {}

  inline WebSocketConfig& WebSocketConfig::SetUri(Uri uri) {
    m_uri = std::move(uri);
    return *this;
  }

  inline WebSocketConfig& WebSocketConfig::SetVersion(std::string version) {
    m_version = std::move(version);
    return *this;
  }

  inline WebSocketConfig& WebSocketConfig::SetProtocols(
      std::vector<std::string> protocols) {
    m_protocols = std::move(protocols);
    return *this;
  }

  inline WebSocketConfig& WebSocketConfig::SetExtensions(
      std::vector<std::string> extensions) {
    m_extensions = std::move(extensions);
    return *this;
  }

  template<typename ChannelType>
  WebSocket<ChannelType>::WebSocket(WebSocketConfig config,
      ChannelBuilder channelBuilder)
      : m_uri{std::move(config.m_uri)},
        m_protocols{std::move(config.m_protocols)},
        m_extensions{std::move(config.m_extensions)},
        m_version{std::move(config.m_version)},
        m_channelBuilder{std::move(channelBuilder)},
        m_randomEngine{static_cast<unsigned int>(std::time(nullptr))} {
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
  IO::SharedBuffer WebSocket<ChannelType>::Read() {
    IO::SharedBuffer payload;
    while(true) {
      int isFinalFragment;
      std::uint8_t opCode;
      int hasMask;
      std::uint32_t maskingKey;
      std::uint64_t payloadLength;
      unsigned char opCodeBuffer[2];
      m_channel->GetReader().Read(reinterpret_cast<char*>(opCodeBuffer),
        sizeof(opCodeBuffer));
      isFinalFragment = opCodeBuffer[0] & (1 << 7);
      opCode = opCodeBuffer[0] & 0x0F;
      hasMask = opCodeBuffer[1] & (1 << 7);
      payloadLength = opCodeBuffer[1] & ~(1 << 7);
      if(payloadLength == 126) {
        std::uint16_t revisedPayloadLength;
        m_channel->GetReader().Read(
          reinterpret_cast<char*>(&revisedPayloadLength),
          sizeof(revisedPayloadLength));
        payloadLength = FromBigEndian(revisedPayloadLength);
      } else if(payloadLength == 127) {
        std::uint64_t revisedPayloadLength;
        m_channel->GetReader().Read(
          reinterpret_cast<char*>(&revisedPayloadLength),
          sizeof(revisedPayloadLength));
        payloadLength = FromBigEndian(revisedPayloadLength);
      }
      if(hasMask) {
        m_channel->GetReader().Read(reinterpret_cast<char*>(&maskingKey),
          sizeof(maskingKey));
      }
      std::size_t cursor = payload.GetSize();
      m_channel->GetReader().Read(Store(payload),
        static_cast<std::size_t>(payloadLength));
      if(hasMask) {
        for(auto i = cursor; i < payloadLength; ++i) {
          payload.GetMutableData()[i] = payload.GetMutableData()[i] ^
            (reinterpret_cast<unsigned char*>(&maskingKey)[
            (i - cursor) % sizeof(maskingKey)]);
        }
      }
      if(isFinalFragment) {
        break;
      }
    }
    return payload;
  }

  template<typename ChannelType>
  template<typename Buffer>
  void WebSocket<ChannelType>::Write(const Buffer& buffer) {
    typename Channel::Writer::Buffer frame;
    std::uint8_t code = (1 << 7) | 1;
    frame.Append(&code, sizeof(code));
    std::uint8_t length = (1 << 7) |
      static_cast<std::uint8_t>(buffer.GetSize());
    frame.Append(&length, sizeof(length));
    std::uint32_t maskingKey = m_randomEngine();
    frame.Append(&maskingKey, sizeof(maskingKey));
    auto size = frame.GetSize();
    frame.Append(buffer);
    for(std::size_t i = 0; i < buffer.GetSize(); ++i) {
      frame.GetMutableData()[i + size] = frame.GetMutableData()[i + size] ^
        (reinterpret_cast<unsigned char*>(&maskingKey)[i % sizeof(maskingKey)]);
    }
    m_channel->GetWriter().Write(frame);
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
      if(!m_extensions.empty()) {
        std::string extensions;
        auto isFirst = true;
        for(auto& extension : m_extensions) {
          if(isFirst) {
            isFirst = false;
          } else {
            extensions += "; ";
          }
          extensions += extension;
        }
        request.Add(HttpHeader{"Sec-WebSocket-Extensions", extensions});
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
    m_frameBuffer = m_parser.GetRemainingBuffer();
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
