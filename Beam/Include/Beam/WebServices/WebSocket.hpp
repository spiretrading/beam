#ifndef BEAM_WEB_SOCKET_HPP
#define BEAM_WEB_SOCKET_HPP
#include <array>
#include <ctime>
#include <functional>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/WebServices/HttpHeader.hpp"
#include "Beam/WebServices/HttpMethod.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponseParser.hpp"
#include "Beam/WebServices/HttpStatusCode.hpp"
#include "Beam/WebServices/HttpVersion.hpp"
#include "Beam/WebServices/Uri.hpp"

namespace Beam {
namespace Details {
  inline std::string compute_sha_digest(std::string_view source) {
    auto sha = CryptoPP::SHA1();
    auto digest = std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE>();
    sha.CalculateDigest(digest.data(), reinterpret_cast<const CryptoPP::byte*>(
      source.data()), source.length());
    return std::string(
      reinterpret_cast<const char*>(digest.data()), CryptoPP::SHA1::DIGESTSIZE);
  }

  inline std::string generate_key() {
    static auto random_pool = CryptoPP::AutoSeededRandomPool();
    static const auto KEY_LENGTH = 16;
    auto random_bytes = std::array<char, KEY_LENGTH>();
    random_pool.GenerateBlock(
      reinterpret_cast<CryptoPP::byte*>(random_bytes.data()), KEY_LENGTH);
    return std::string(random_bytes.data(), KEY_LENGTH);
  }
}

  /** Contains the configuration needed to construct a WebSocket. */
  class WebSocketConfig {
    public:

      /** Constructs a WebSocketConfig with default values. */
      WebSocketConfig();

      /** Sets the URI to connect to. */
      WebSocketConfig& set_uri(const Uri& uri);

      /** Sets the web socket version. */
      WebSocketConfig& set_version(const std::string& version);

      /** Sets the list of protocols. */
      WebSocketConfig& set_protocols(const std::vector<std::string>& protocols);

      /** Sets the list of extensions. */
      WebSocketConfig& set_extensions(
        const std::vector<std::string>& extensions);

    private:
      template<typename C> requires IsChannel<dereference_t<C>>
        friend class WebSocket;
      Uri m_uri;
      std::string m_version;
      std::vector<std::string> m_protocols;
      std::vector<std::string> m_extensions;
  };

  /**
   * Implements a WebSocket connection to a server.
   * @tparam C The type Channel used to connect to the server.
   */
  template<typename C> requires IsChannel<dereference_t<C>>
  class WebSocket {
    struct ServerTag {};
    public:

      /** The type of Channel used to connect to the server. */
      using Channel = dereference_t<C>;

      /**
       * The factory used to build Channels.
       * @param uri The URI to connect to.
       * @return A Channel able to connect to the specified <i>uri</i>.
       */
      using ChannelBuilder = std::function<C (const Uri& uri)>;

      /**
       * Constructs a WebSocket.
       * @param config The config used to initialize this socket.
       * @param channel_builder Constructs the Channel used to connect to the
       *        server.
       */
      WebSocket(WebSocketConfig config, ChannelBuilder channel_builder);

      /**
       * Constructs a WebSocket operating in server-mode for internal use only.
       * @param channel The existing Channel to adapt.
       * @param tag Internal use only.
       */
      template<Initializes<C> CF>
      WebSocket(CF&& channel, ServerTag tag);

      ~WebSocket();

      /** Returns the Uri this socket connects to. */
      const Uri& get_uri() const;

      /** Reads the next frame from the web socket. */
      SharedBuffer read();

      /**
       * Writes to the web socket.
       * @param buffer The buffer to write.
       */
      template<IsConstBuffer B>
      void write(const B& buffer);

      void close();

    private:
      template<typename S> requires IsServerConnection<dereference_t<S>>
      friend class HttpServer;
      bool m_is_server_mode;
      Uri m_uri;
      std::vector<std::string> m_protocols;
      std::vector<std::string> m_extensions;
      std::string m_version;
      ChannelBuilder m_channel_builder;
      HttpResponseParser m_parser;
      local_ptr_t<C> m_channel;
      std::mt19937 m_random_engine;
      SharedBuffer m_frame_buffer;
      OpenState m_open_state;

      void open();
  };

  template<typename F>
  WebSocket(WebSocketConfig, F) ->
    WebSocket<std::invoke_result_t<F, const Uri&>>;

  inline WebSocketConfig::WebSocketConfig()
    : m_version("13") {}

  inline WebSocketConfig& WebSocketConfig::set_uri(const Uri& uri) {
    m_uri = uri;
    return *this;
  }

  inline WebSocketConfig& WebSocketConfig::set_version(
      const std::string& version) {
    m_version = version;
    return *this;
  }

  inline WebSocketConfig& WebSocketConfig::set_protocols(
      const std::vector<std::string>& protocols) {
    m_protocols = protocols;
    return *this;
  }

  inline WebSocketConfig& WebSocketConfig::set_extensions(
      const std::vector<std::string>& extensions) {
    m_extensions = extensions;
    return *this;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocket<C>::WebSocket(
      WebSocketConfig config, ChannelBuilder channel_builder)
      : m_is_server_mode(false),
        m_uri(std::move(config.m_uri)),
        m_protocols(std::move(config.m_protocols)),
        m_extensions(std::move(config.m_extensions)),
        m_version(std::move(config.m_version)),
        m_channel_builder(std::move(channel_builder)),
        m_random_engine(static_cast<unsigned int>(std::time(nullptr))) {
    if(m_uri.get_port() == 0) {
      if(m_uri.get_scheme() == "http" || m_uri.get_scheme() == "ws") {
        m_uri.set_port(80);
      } else if(m_uri.get_scheme() == "https" || m_uri.get_scheme() == "wss") {
        m_uri.set_port(443);
      }
    }
    open();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<Initializes<C> CF>
  WebSocket<C>::WebSocket(CF&& channel, ServerTag)
    : m_is_server_mode(true),
      m_channel(std::forward<CF>(channel)) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocket<C>::~WebSocket() {
    close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  const Uri& WebSocket<C>::get_uri() const {
    return m_uri;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  SharedBuffer WebSocket<C>::read() {
    auto payload = SharedBuffer();
    while(true) {
      auto op_code_buffer = std::array<unsigned char, 2>();
      Beam::read(m_channel->get_reader(), out(op_code_buffer));
      auto is_final_fragment = op_code_buffer[0] & (1 << 7);
      auto op_code = op_code_buffer[0] & 0x0F;
      auto has_mask = op_code_buffer[1] & (1 << 7);
      auto masking_key = std::uint32_t();
      auto payload_length = op_code_buffer[1] & ~(1 << 7);
      if(payload_length == 126) {
        auto revised_payload_length = std::uint16_t();
        Beam::read(m_channel->get_reader(), out(revised_payload_length));
        payload_length = boost::endian::big_to_native(revised_payload_length);
      } else if(payload_length == 127) {
        auto revised_payload_length = std::uint64_t();
        Beam::read(m_channel->get_reader(), out(revised_payload_length));
        payload_length = boost::endian::big_to_native(revised_payload_length);
      }
      if(has_mask) {
        Beam::read(m_channel->get_reader(), out(masking_key));
      }
      auto cursor = payload.get_size();
      if(payload_length != 0) {
        m_channel->get_reader().read(out(payload), payload_length);
      }
      if(has_mask) {
        for(auto i = cursor; i < payload_length; ++i) {
          payload.get_mutable_data()[i] = payload.get_mutable_data()[i] ^
            (reinterpret_cast<unsigned char*>(&masking_key)[
              (i - cursor) % sizeof(masking_key)]);
        }
      }
      if(is_final_fragment) {
        break;
      }
    }
    return payload;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<IsConstBuffer B>
  void WebSocket<C>::write(const B& buffer) {
    static const auto MAX_PAYLOAD_LENGTH = std::size_t(125);
    static const auto MAX_TWO_BYTE_PAYLOAD_LENGTH = std::size_t(1 << 16);
    auto size = buffer.get_size();
    auto data = buffer.get_data();
    auto frame = SharedBuffer();
    append(frame, std::uint8_t((1 << 7) | 1));
    auto payload_length = [&] {
      if(size <= MAX_PAYLOAD_LENGTH) {
        return static_cast<std::uint8_t>(size);
      } else if(size <= MAX_TWO_BYTE_PAYLOAD_LENGTH) {
        return std::uint8_t{126};
      } else {
        return std::uint8_t{127};
      }
    }();
    if(!m_is_server_mode) {
      payload_length |= (1 << 7);
    }
    append(frame, payload_length);
    if(size > MAX_PAYLOAD_LENGTH) {
      if(size <= MAX_TWO_BYTE_PAYLOAD_LENGTH) {
        auto extended_payload_length =
          boost::endian::native_to_big(static_cast<std::uint16_t>(size));
        append(frame, extended_payload_length);
      } else {
        auto extended_payload_length =
          boost::endian::native_to_big(static_cast<std::uint64_t>(size));
        append(frame, extended_payload_length);
      }
    }
    if(m_is_server_mode) {
      append(frame, data, size);
    } else {
      auto masking_key = std::uint32_t(m_random_engine());
      append(frame, masking_key);
      auto frame_size = frame.get_size();
      append(frame, data, size);
      for(auto i = std::size_t(0); i < size; ++i) {
        auto index = i + frame_size;
        frame.get_mutable_data()[index] = frame.get_mutable_data()[index] ^
          (reinterpret_cast<unsigned char*>(&masking_key)[
            i % sizeof(masking_key)]);
      }
    }
    m_channel->get_writer().write(frame);
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void WebSocket<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void WebSocket<C>::open() {
    static constexpr auto MAGIC_TOKEN = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    try {
      auto request =
        HttpRequest(HttpVersion::version_1_1(), HttpMethod::GET, m_uri);
      request.add(HttpHeader("Upgrade", "websocket"));
      request.add(HttpHeader("Connection", "Upgrade"));
      auto key = Details::generate_key();
      auto base64_key = encode_base64(from<SharedBuffer>(key));
      request.add(HttpHeader("Sec-WebSocket-Key", base64_key));
      if(!m_protocols.empty()) {
      auto protocols = std::string();
        auto is_first = true;
        for(auto& protocol : m_protocols) {
          if(is_first) {
            is_first = false;
          } else {
            protocols += ", ";
          }
          protocols += protocol;
        }
        request.add(HttpHeader("Sec-WebSocket-Protocol", protocols));
      }
      if(!m_extensions.empty()) {
        auto extensions = std::string();
        auto is_first = true;
        for(auto& extension : m_extensions) {
          if(is_first) {
            is_first = false;
          } else {
            extensions += "; ";
          }
          extensions += extension;
        }
        request.add(HttpHeader("Sec-WebSocket-Extensions", extensions));
      }
      if(!m_version.empty()) {
        request.add(HttpHeader("Sec-WebSocket-Version", m_version));
      }
      auto send_buffer = SharedBuffer();
      request.encode(out(send_buffer));
      m_channel = m_channel_builder(m_uri);
      m_channel->get_writer().write(send_buffer);
      auto receive_buffer = SharedBuffer();
      while(true) {
        m_channel->get_reader().read(out(receive_buffer));
        m_parser.feed(std::string_view(
          receive_buffer.get_data(), receive_buffer.get_size()));
        auto response = m_parser.get_next_response();
        reset(receive_buffer);
        if(response) {
          if(response->get_status_code() !=
              HttpStatusCode::SWITCHING_PROTOCOLS) {
            boost::throw_with_location(
              ConnectException("Invalid status code: " +
                get_reason_phrase(response->get_status_code())));
          }
          auto accept_header = response->get_header("Sec-WebSocket-Accept");
          if(!accept_header.is_initialized()) {
            boost::throw_with_location(ConnectException("Invalid accept key."));
          }
          auto accept_token = encode_base64(from<SharedBuffer>(
            Details::compute_sha_digest(base64_key + MAGIC_TOKEN)));
          if(accept_token != *accept_header) {
            boost::throw_with_location(ConnectException("Invalid accept key."));
          }
          break;
        }
      }
    } catch(const std::exception&) {
      close();
      throw;
    }
    m_frame_buffer = m_parser.get_remaining_buffer();
  }
}

#endif
