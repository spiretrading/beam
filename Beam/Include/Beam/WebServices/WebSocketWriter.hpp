#ifndef BEAM_WEB_SOCKET_WRITER_HPP
#define BEAM_WEB_SOCKET_WRITER_HPP
#include <memory>
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {

  /**
   * Provides the Writer interface to a WebSocket.
   * @tparam T The type of WebSocket to adapt.
   */
  template<typename T>
  class WebSocketWriter;

  template<typename C> requires IsChannel<dereference_t<C>>
  class WebSocketWriter<WebSocket<C>> {
    public:

      /** The type of WebSocket to write to. */
      using WebSocket = Beam::WebSocket<C>;

      template<IsConstBuffer B>
      void write(const B& data);

    private:
      template<typename S> requires IsChannel<dereference_t<S>>
      friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;

      WebSocketWriter(std::shared_ptr<WebSocket> socket);
      WebSocketWriter(const WebSocketWriter&) = delete;
      WebSocketWriter& operator =(const WebSocketWriter&) = delete;
  };

  template<typename C> requires IsChannel<dereference_t<C>>
  template<IsConstBuffer B>
  void WebSocketWriter<WebSocket<C>>::write(const B& data) {
    m_socket->write(data);
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocketWriter<WebSocket<C>>::WebSocketWriter(
    std::shared_ptr<WebSocket> socket)
    : m_socket(std::move(socket)) {}
}

#endif
