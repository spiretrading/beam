#ifndef BEAM_WEB_SOCKET_CHANNEL_HPP
#define BEAM_WEB_SOCKET_CHANNEL_HPP
#include <memory>
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/WebServices/Uri.hpp"
#include "Beam/WebServices/WebSocket.hpp"
#include "Beam/WebServices/WebSocketConnection.hpp"
#include "Beam/WebServices/WebSocketReader.hpp"
#include "Beam/WebServices/WebSocketWriter.hpp"

namespace Beam {

  /**
   * Implements the Channel interface for a WebSocket.
   * @tparam C The type of Channel used by the WebSocket.
   */
  template<typename C> requires IsChannel<dereference_t<C>>
  class WebSocketChannel {
    public:
      using WebSocket = Beam::WebSocket<C>;
      using Identifier = Uri;
      using Connection = WebSocketConnection<WebSocket>;
      using Reader = WebSocketReader<WebSocket>;
      using Writer = WebSocketWriter<WebSocket>;

      /**
       * Constructs a WebSocketChannel.
       * @param config The config used to initialize the WebSocket.
       * @param channel_builder Constructs the Channel used to connect to the
       *        server.
       */
      WebSocketChannel(WebSocketConfig config,
        typename WebSocket::ChannelBuilder channel_builder);

      /**
       * Constructs a WebSocketChannel from an existing WebSocket.
       * @param web_socket The WebSocket to adapt.
       */
      explicit WebSocketChannel(std::unique_ptr<WebSocket> web_socket);

      /** Returns the underlying WebSocket. */
      WebSocket& get_socket();

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      std::shared_ptr<WebSocket> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      WebSocketChannel(const WebSocketChannel&) = delete;
      WebSocketChannel& operator =(const WebSocketChannel&) = delete;
  };

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocketChannel<C>::WebSocketChannel(WebSocketConfig config,
    typename WebSocket::ChannelBuilder channel_builder)
    : m_socket(std::make_shared<WebSocket>(
        std::move(config), std::move(channel_builder))),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocketChannel<C>::WebSocketChannel(std::unique_ptr<WebSocket> web_socket)
    : m_socket(std::move(web_socket)),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  typename WebSocketChannel<C>::WebSocket& WebSocketChannel<C>::get_socket() {
    return *m_socket;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  const typename WebSocketChannel<C>::Identifier&
      WebSocketChannel<C>::get_identifier() const {
    return m_socket->get_uri();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename WebSocketChannel<C>::Connection&
      WebSocketChannel<C>::get_connection() {
    return m_connection;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename WebSocketChannel<C>::Reader& WebSocketChannel<C>::get_reader() {
    return m_reader;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  typename WebSocketChannel<C>::Writer& WebSocketChannel<C>::get_writer() {
    return m_writer;
  }
}

#endif
