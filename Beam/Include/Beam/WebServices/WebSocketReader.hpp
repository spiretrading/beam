#ifndef BEAM_WEB_SOCKET_READER_HPP
#define BEAM_WEB_SOCKET_READER_HPP
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/WebServices.hpp"
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {
namespace WebServices {

  /** Implements the Reader interface for a WebSocket. */
  template<typename S>
  class WebSocketReader {
    public:

      /** The type of WebSocket to read from. */
      using WebSocket = S;

      bool IsDataAvailable() const;

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      template<typename> friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;
      boost::optional<IO::BufferReader<IO::SharedBuffer>> m_reader;

      WebSocketReader(std::shared_ptr<WebSocket> socket);
      void ReadFromWebSocket();
  };

  template<typename S>
  bool WebSocketReader<S>::IsDataAvailable() const {
    return m_reader && m_reader->IsDataAvailable();
  }

  template<typename S>
  template<typename Buffer>
  std::size_t WebSocketReader<S>::Read(Out<Buffer> destination) {
    if(!m_reader) {
      ReadFromWebSocket();
    }
    try {
      return m_reader->Read(Store(destination));
    } catch(const IO::EndOfFileException&) {
      m_reader.reset();
      return Read(Store(destination));
    }
  }

  template<typename S>
  std::size_t WebSocketReader<S>::Read(char* destination, std::size_t size) {
    if(!m_reader) {
      ReadFromWebSocket();
    }
    try {
      return m_reader->Read(destination, size);
    } catch(const IO::EndOfFileException&) {
      m_reader.reset();
      return Read(destination, size);
    }
  }

  template<typename S>
  template<typename Buffer>
  std::size_t WebSocketReader<S>::Read(Out<Buffer> destination,
      std::size_t size) {
    if(!m_reader) {
      ReadFromWebSocket();
    }
    try {
      return m_reader->Read(Store(destination), size);
    } catch(const IO::EndOfFileException&) {
      m_reader.reset();
      return Read(Store(destination), size);
    }
  }

  template<typename S>
  WebSocketReader<S>::WebSocketReader(std::shared_ptr<WebSocket> socket)
    : m_socket(socket) {}

  template<typename S>
  void WebSocketReader<S>::ReadFromWebSocket() {
    m_reader.emplace(m_socket->Read());
  }
}

  template<typename S>
  struct ImplementsConcept<WebServices::WebSocketReader<S>, IO::Reader> :
    std::true_type {};
}

#endif
