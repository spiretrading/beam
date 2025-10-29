#ifndef BEAM_WEB_SOCKET_CONNECTION_HPP
#define BEAM_WEB_SOCKET_CONNECTION_HPP
#include <memory>
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {

  /**
   * Provides a Connection interface for a WebSocket.
   * @tparam T The type of WebSocket to adapt.
   */
  template<typename T>
  class WebSocketConnection;

  template<typename C> requires IsChannel<dereference_t<C>>
  class WebSocketConnection<WebSocket<C>> {
    public:

      /** The type of WebSocket to connect. */
      using WebSocket = Beam::WebSocket<C>;

      ~WebSocketConnection();

      void close();

    private:
      template<typename S> requires IsChannel<dereference_t<S>>
      friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;

      WebSocketConnection(const std::shared_ptr<WebSocket>& socket);
      WebSocketConnection(const WebSocketConnection&) = delete;
      WebSocketConnection& operator =(const WebSocketConnection&) = delete;
  };

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocketConnection<WebSocket<C>>::~WebSocketConnection() {
    close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  void WebSocketConnection<WebSocket<C>>::close() {
    m_socket->close();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocketConnection<WebSocket<C>>::WebSocketConnection(
    const std::shared_ptr<WebSocket>& socket)
    : m_socket(socket) {}
}

#endif
