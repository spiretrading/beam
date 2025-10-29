#ifndef BEAM_WEB_SOCKET_READER_HPP
#define BEAM_WEB_SOCKET_READER_HPP
#include <boost/optional/optional.hpp>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {

  /**
   * Implements the Reader interface for a WebSocket.
   * @tparam T The type of WebSocket to adapt.
   */
  template<typename T>
  class WebSocketReader;

  template<typename C> requires IsChannel<dereference_t<C>>
  class WebSocketReader<WebSocket<C>> {
    public:

      /** The type of WebSocket to read from. */
      using WebSocket = Beam::WebSocket<C>;

      bool poll() const;
      template<IsBuffer B>
      std::size_t read(Out<B> destination, std::size_t size = -1);

    private:
      template<typename S> requires IsChannel<dereference_t<S>>
      friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;
      boost::optional<BufferReader<SharedBuffer>> m_reader;

      WebSocketReader(std::shared_ptr<WebSocket> socket);
      WebSocketReader(const WebSocketReader&) = delete;
      WebSocketReader& operator =(const WebSocketReader&) = delete;
      void read_from_web_socket();
  };

  template<typename C> requires IsChannel<dereference_t<C>>
  bool WebSocketReader<WebSocket<C>>::poll() const {
    return m_reader && m_reader->poll();
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  template<IsBuffer B>
  std::size_t WebSocketReader<WebSocket<C>>::read(
      Out<B> destination, std::size_t size) {
    if(!m_reader) {
      read_from_web_socket();
    }
    try {
      return m_reader->read(out(destination), size);
    } catch(const EndOfFileException&) {
      m_reader.reset();
      return read(out(destination), size);
    }
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  WebSocketReader<WebSocket<C>>::WebSocketReader(
    std::shared_ptr<WebSocket> socket)
    : m_socket(std::move(socket)) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  void WebSocketReader<WebSocket<C>>::read_from_web_socket() {
    m_reader.emplace(m_socket->read());
  }
}

#endif
