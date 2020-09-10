#ifndef BEAM_WEB_SOCKET_HPP
#define BEAM_WEB_SOCKET_HPP
#include <array>
#include <ctime>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {
namespace Details {
  inline std::string ComputeShaDigest(const std::string& source) {
    auto sha = CryptoPP::SHA1();
    auto digest = std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE>();
    sha.CalculateDigest(digest.data(), reinterpret_cast<const CryptoPP::byte*>(
      source.c_str()), source.length());
    return std::string(reinterpret_cast<const char*>(digest.data()),
      CryptoPP::SHA1::DIGESTSIZE);
  }

  inline std::string GenerateKey() {
    static auto randomPool = CryptoPP::AutoSeededRandomPool();
    static constexpr auto KEY_LENGTH = 16;
    auto randomBytes = std::array<char, KEY_LENGTH>();
    randomPool.GenerateBlock(
      reinterpret_cast<CryptoPP::byte*>(randomBytes.data()), KEY_LENGTH);
    return std::string(randomBytes.data(), KEY_LENGTH);
  }
}

  /** Contains the configuration needed to construct a WebSocket. */
  class WebSocketConfig {
    public:

      /** Constructs a WebSocketConfig with default values. */
      WebSocketConfig();

      /** Sets the URI to connect to. */
      WebSocketConfig& SetUri(Uri uri);

      /** Sets the websocket version. */
      WebSocketConfig& SetVersion(std::string version);

      /** Sets the list of protocols. */
      WebSocketConfig& SetProtocols(std::vector<std::string> protocols);

      /** Sets the list of extensions. */
      WebSocketConfig& SetExtensions(std::vector<std::string> extensions);

    private:
      template<typename> friend class WebSocket;
      Uri m_uri;
      std::string m_version;
      std::vector<std::string> m_protocols;
      std::vector<std::string> m_extensions;
  };

  /**
   * Implements a WebSocket connection to a server.
   * @param <C> The type Channel used to connect to the server.
   */
  template<typename C>
  class WebSocket {
    struct ServerTag{};
    public:

      /** The type of Channel used to connect to the server. */
      using Channel = GetTryDereferenceType<C>;

      /**
       * The factory used to build Channels.
       * @param uri The URI to connect to.
       * @return A Channel able to connect to the specified <i>uri</i>.
       */
      using ChannelBuilder = std::function<C (const Uri& uri)>;

      /**
       * Constructs a WebSocket.
       * @param config The config used to initialize this socket.
       * @param channelBuilder Builds the Channel used to connect to the server.
       */
      WebSocket(WebSocketConfig config, ChannelBuilder channelBuilder);

      /**
       * Constructs a WebSocket operating in server-mode for internal use only.
       * @param channel The existing Channel to adapt.
       * @param tag Internal use only.
       */
      template<typename CF>
      WebSocket(CF&& channel, ServerTag tag);

      ~WebSocket();

      /** Returns the Uri this socket connects to. */
      const Uri& GetUri() const;

      /** Reads the next frame from the web socket. */
      IO::SharedBuffer Read();

      /**
       * Writes to the web socket.
       * @param data The raw data to write.
       * @param size The number of bytes to write.
       */
      void Write(const void* data, std::size_t size);

      /**
       * Writes to the web socket.
       * @param buffer The buffer to write.
       */
      template<typename Buffer>
      void Write(const Buffer& buffer);

      void Close();

    private:
      template<typename> friend class HttpServer;
      bool m_isServerMode;
      Uri m_uri;
      std::vector<std::string> m_protocols;
      std::vector<std::string> m_extensions;
      std::string m_version;
      ChannelBuilder m_channelBuilder;
      HttpResponseParser m_parser;
      GetOptionalLocalPtr<C> m_channel;
      std::mt19937 m_randomEngine;
      typename Channel::Reader::Buffer m_frameBuffer;
      IO::OpenState m_openState;

      void Open();
  };

  inline WebSocketConfig::WebSocketConfig()
    : m_version("13") {}

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

  template<typename C>
  WebSocket<C>::WebSocket(WebSocketConfig config, ChannelBuilder channelBuilder)
      : m_isServerMode(false),
        m_uri(std::move(config.m_uri)),
        m_protocols(std::move(config.m_protocols)),
        m_extensions(std::move(config.m_extensions)),
        m_version(std::move(config.m_version)),
        m_channelBuilder(std::move(channelBuilder)),
        m_randomEngine(static_cast<unsigned int>(std::time(nullptr))) {
    if(m_uri.GetPort() == 0) {
      if(m_uri.GetScheme() == "http" || m_uri.GetScheme() == "ws") {
        m_uri.SetPort(80);
      } else if(m_uri.GetScheme() == "https" || m_uri.GetScheme() == "wss") {
        m_uri.SetPort(443);
      }
    }
    Open();
  }

  template<typename C>
  template<typename CF>
  WebSocket<C>::WebSocket(CF&& channel, ServerTag)
    : m_isServerMode(true),
      m_channel(std::forward<CF>(channel)) {}

  template <typename C>
  WebSocket<C>::~WebSocket() {
    Close();
  }

  template<typename C>
  const Uri& WebSocket<C>::GetUri() const {
    return m_uri;
  }

  template<typename C>
  IO::SharedBuffer WebSocket<C>::Read() {
    auto payload = IO::SharedBuffer();
    while(true) {
      auto opCodeBuffer = std::array<unsigned char, 2>();
      m_channel->GetReader().Read(reinterpret_cast<char*>(opCodeBuffer.data()),
        opCodeBuffer.size());
      auto isFinalFragment = 0;
      auto opCode = std::uint8_t();
      auto hasMask = 0;
      auto maskingKey = std::uint32_t();
      auto payloadLength = std::uint64_t();
      isFinalFragment = opCodeBuffer[0] & (1 << 7);
      opCode = opCodeBuffer[0] & 0x0F;
      hasMask = opCodeBuffer[1] & (1 << 7);
      payloadLength = opCodeBuffer[1] & ~(1 << 7);
      if(payloadLength == 126) {
        auto revisedPayloadLength = std::uint16_t();
        m_channel->GetReader().Read(
          reinterpret_cast<char*>(&revisedPayloadLength),
          sizeof(revisedPayloadLength));
        payloadLength = FromBigEndian(revisedPayloadLength);
      } else if(payloadLength == 127) {
        auto revisedPayloadLength = std::uint64_t();
        m_channel->GetReader().Read(
          reinterpret_cast<char*>(&revisedPayloadLength),
          sizeof(revisedPayloadLength));
        payloadLength = FromBigEndian(revisedPayloadLength);
      }
      if(hasMask) {
        m_channel->GetReader().Read(reinterpret_cast<char*>(&maskingKey),
          sizeof(maskingKey));
      }
      auto cursor = payload.GetSize();
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

  template<typename C>
  void WebSocket<C>::Write(const void* data, std::size_t size) {
    static constexpr auto MAX_PAYLOAD_LENGTH = std::size_t(125);
    static constexpr auto MAX_TWO_BYTE_PAYLOAD_LENGTH = std::size_t(1 << 16);
    auto frame = typename Channel::Writer::Buffer();
    auto code = std::uint8_t((1 << 7) | 1);
    frame.Append(&code, sizeof(code));
    auto payloadLength =
      [&] {
        if(size <= MAX_PAYLOAD_LENGTH) {
          return static_cast<std::uint8_t>(size);
        } else if(size <= MAX_TWO_BYTE_PAYLOAD_LENGTH) {
          return std::uint8_t{126};
        } else {
          return std::uint8_t{127};
        }
      }();
    if(!m_isServerMode) {
      payloadLength |= (1 << 7);
    }
    frame.Append(&payloadLength, sizeof(payloadLength));
    if(size > MAX_PAYLOAD_LENGTH) {
      if(size <= MAX_TWO_BYTE_PAYLOAD_LENGTH) {
        auto extendedPayloadLength = ToBigEndian(
          static_cast<std::uint16_t>(size));
        frame.Append(&extendedPayloadLength, sizeof(extendedPayloadLength));
      } else {
        auto extendedPayloadLength = ToBigEndian(
          static_cast<std::uint64_t>(size));
        frame.Append(&extendedPayloadLength, sizeof(extendedPayloadLength));
      }
    }
    if(m_isServerMode) {
      frame.Append(data, size);
    } else {
      auto maskingKey = std::uint32_t(m_randomEngine());
      frame.Append(&maskingKey, sizeof(maskingKey));
      auto frameSize = frame.GetSize();
      frame.Append(data, size);
      for(auto i = std::size_t(0); i < size; ++i) {
        auto index = i + frameSize;
        frame.GetMutableData()[index] = frame.GetMutableData()[index] ^
          (reinterpret_cast<unsigned char*>(&maskingKey)[
          i % sizeof(maskingKey)]);
      }
    }
    m_channel->GetWriter().Write(frame);
  }

  template<typename C>
  template<typename Buffer>
  void WebSocket<C>::Write(const Buffer& buffer) {
    Write(buffer.GetData(), buffer.GetSize());
  }

  template<typename C>
  void WebSocket<C>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_channel->GetConnection().Close();
    m_openState.Close();
  }

  template<typename C>
  void WebSocket<C>::Open() {
    static constexpr auto MAGIC_TOKEN = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    try {
      auto request = HttpRequest(HttpVersion::Version1_1(), HttpMethod::GET,
        m_uri);
      request.Add(HttpHeader{"Upgrade", "websocket"});
      request.Add(HttpHeader{"Connection", "Upgrade"});
      auto key = Details::GenerateKey();
      auto base64Key = IO::Base64Encode(
        IO::BufferFromString<IO::SharedBuffer>(key));
      request.Add(HttpHeader{"Sec-WebSocket-Key", base64Key});
      if(!m_protocols.empty()) {
        auto protocols = std::string();
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
        auto extensions = std::string();
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
      auto sendBuffer = typename Channel::Writer::Buffer();
      auto sendStream =
        IO::BufferOutputStream<typename Channel::Writer::Buffer>(
        Ref(sendBuffer));
      sendStream << request;
      sendStream.flush();
      m_channel = m_channelBuilder(m_uri);
      m_channel->GetWriter().Write(sendBuffer);
      auto receiveBuffer = typename Channel::Reader::Buffer();
      while(true) {
        m_channel->GetReader().Read(Store(receiveBuffer));
        m_parser.Feed(receiveBuffer.GetData(), receiveBuffer.GetSize());
        auto response = m_parser.GetNextResponse();
        receiveBuffer.Reset();
        if(response) {
          if(response->GetStatusCode() != HttpStatusCode::SWITCHING_PROTOCOLS) {
            BOOST_THROW_EXCEPTION(IO::ConnectException("Invalid status code: " +
              GetReasonPhrase(response->GetStatusCode())));
          }
          auto acceptHeader = response->GetHeader("Sec-WebSocket-Accept");
          if(!acceptHeader.is_initialized()) {
            BOOST_THROW_EXCEPTION(IO::ConnectException("Invalid accept key."));
          }
          auto acceptToken = IO::Base64Encode(
            IO::BufferFromString<IO::SharedBuffer>(Details::ComputeShaDigest(
            base64Key + MAGIC_TOKEN)));
          if(acceptToken != *acceptHeader) {
            BOOST_THROW_EXCEPTION(IO::ConnectException{"Invalid accept key."});
          }
          break;
        }
      }
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
    m_frameBuffer = m_parser.GetRemainingBuffer();
  }
}

#endif
