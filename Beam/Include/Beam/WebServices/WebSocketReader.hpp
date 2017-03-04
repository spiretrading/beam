#ifndef BEAM_WEBSOCKETREADER_HPP
#define BEAM_WEBSOCKETREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/WebServices/WebServices.hpp"
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocketReader
      \brief Implements the Reader interface for a WebSocket.
   */
  template<typename WebSocketType>
  class WebSocketReader : private boost::noncopyable {
    public:

      //! The type of WebSocket to read from.
      using WebSocket = WebSocketType;
      using Buffer = IO::SharedBuffer;

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      template<typename> friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;
      boost::optional<IO::BufferReader<IO::SharedBuffer>> m_reader;

      WebSocketReader(const std::shared_ptr<WebSocket>& socket);
      void ReadFromWebSocket();
  };

  template<typename WebSocketType>
  bool WebSocketReader<WebSocketType>::IsDataAvailable() const {
    return m_reader.is_initialized() && m_reader->IsDataAvailable();
  }

  template<typename WebSocketType>
  std::size_t WebSocketReader<WebSocketType>::Read(Out<Buffer> destination) {
    if(!m_reader.is_initialized()) {
      ReadFromWebSocket();
    }
    try {
      return m_reader->Read(Store(destination));
    } catch(const IO::EndOfFileException&) {
      m_reader.reset();
      return Read(Store(destination));
    }
  }

  template<typename WebSocketType>
  std::size_t WebSocketReader<WebSocketType>::Read(char* destination,
      std::size_t size) {
    if(!m_reader.is_initialized()) {
      ReadFromWebSocket();
    }
    try {
      return m_reader->Read(destination, size);
    } catch(const IO::EndOfFileException&) {
      m_reader.reset();
      return Read(destination, size);
    }
  }

  template<typename WebSocketType>
  std::size_t WebSocketReader<WebSocketType>::Read(Out<Buffer> destination,
      std::size_t size) {
    if(!m_reader.is_initialized()) {
      ReadFromWebSocket();
    }
    try {
      return m_reader->Read(Store(destination), size);
    } catch(const IO::EndOfFileException&) {
      m_reader.reset();
      return Read(Store(destination), size);
    }
  }

  template<typename WebSocketType>
  WebSocketReader<WebSocketType>::WebSocketReader(
      const std::shared_ptr<WebSocket>& socket)
      : m_socket{socket} {}

  template<typename WebSocketType>
  void WebSocketReader<WebSocketType>::ReadFromWebSocket() {
    m_reader.emplace(m_socket->Read());
  }
}

  template<typename WebSocketType>
  struct ImplementsConcept<WebServices::WebSocketReader<WebSocketType>,
      IO::Reader<typename WebServices::WebSocketReader<WebSocketType>::Buffer>>
      : std::true_type {};
}

#endif
