#ifndef BEAM_WEBSOCKETWRITER_HPP
#define BEAM_WEBSOCKETWRITER_HPP
#include <boost/noncopyable.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include "Beam/IO/Writer.hpp"
#include "Beam/WebServices/WebServices.hpp"
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocketWriter
      \brief Provides the Writer interface to a WebSocket.
   */
  template<typename WebSocketType>
  class WebSocketWriter : private boost::noncopyable {
    public:

      //! The type of WebSocket to write to.
      using WebSocket = WebSocketType;
      using Buffer = IO::SharedBuffer;

      void Write(const void* data, std::size_t size);

      void Write(const Buffer& data);

    private:
      template<typename> friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;

      WebSocketWriter(const std::shared_ptr<WebSocket>& socket);
  };

  template<typename WebSocketType>
  void WebSocketWriter<WebSocketType>::Write(const void* data,
      std::size_t size) {
    m_socket->Write(data, size);
  }

  template<typename WebSocketType>
  void WebSocketWriter<WebSocketType>::Write(const Buffer& data) {
    Write(data.GetData(), data.GetSize());
  }

  template<typename WebSocketType>
  WebSocketWriter<WebSocketType>::WebSocketWriter(
      const std::shared_ptr<WebSocket>& socket)
      : m_socket{socket} {}
}

  template<typename WebSocketType>
  struct ImplementsConcept<WebServices::WebSocketWriter<WebSocketType>,
      IO::Writer<typename WebServices::WebSocketWriter<WebSocketType>::Buffer>>
      : std::true_type {};
}

#endif
