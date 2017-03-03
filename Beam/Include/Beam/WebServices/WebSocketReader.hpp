#ifndef BEAM_WEBSOCKETREADER_HPP
#define BEAM_WEBSOCKETREADER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Reader.hpp"
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
      using Buffer = WebSocket::Channel::Reader::Buffer;

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      friend class WebSocketChannel<WebSocket>;
      std::shared_ptr<WebSocket> m_socket;

      WebSocketReader(const std::shared_ptr<WebSocket>& socket);
  };

  template<typename WebSocketType>
  bool WebSocketReader<WebSocketType>::IsDataAvailable() const {
    return false;
  }

  template<typename WebSocketType>
  std::size_t WebSocketReader<WebSocketType>::Read(Out<Buffer> destination) {
    return 0;
  }

  template<typename WebSocketType>
  std::size_t WebSocketReader<WebSocketType>::::Read(char* destination,
      std::size_t size) {
    return 0;
  }

  template<typename WebSocketType>
  std::size_t WebSocketReader<WebSocketType>::Read(Out<Buffer> destination,
      std::size_t size) {
    return 0;
  }

  template<typename WebSocketType>
  WebSocketReader<WebSocketType>::WebSocketReader(
      const std::shared_ptr<WebSocket>& socket)
      : m_socket{socket} {}
}

  template<typename WebSocketType>
  struct ImplementsConcept<WebServices::WebSocketReader<WebSocketType>,
      IO::Reader<typename WebServices::WebSocketReader<WebSocketType>::Buffer>>
      : std::true_type {};
}

#endif
