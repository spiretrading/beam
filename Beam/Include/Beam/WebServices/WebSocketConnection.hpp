#ifndef BEAM_WEBSOCKETCONNECTION_HPP
#define BEAM_WEBSOCKETCONNECTION_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/WebServices/WebServices.hpp"
#include "Beam/WebServices/WebSocket.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocketConnection
      \brief Provides a Connection interface for a WebSocket.
      \tparam WebSocketType The type of WebSocket to adapt.
   */
  template<typename WebSocketType>
  class WebSocketConnection : private boost::noncopyable {
    public:

      //! The type of WebSocket to connect.
      using WebSocket = WebSocketType;

      ~WebSocketConnection();

      void Close();

    private:
      template<typename> friend class WebSocketChannel;
      std::shared_ptr<WebSocket> m_socket;

      WebSocketConnection(const std::shared_ptr<WebSocket>& socket);
  };

  template<typename WebSocketType>
  WebSocketConnection<WebSocketType>::~WebSocketConnection() {
    Close();
  }

  template<typename WebSocketType>
  void WebSocketConnection<WebSocketType>::Close() {
    m_socket->Close();
  }

  template<typename WebSocketType>
  WebSocketConnection<WebSocketType>::WebSocketConnection(
      const std::shared_ptr<WebSocket>& socket)
      : m_socket{socket} {}
}

  template<typename WebSocketType>
  struct ImplementsConcept<WebServices::WebSocketConnection<WebSocketType>,
    IO::Connection> : std::true_type {};
}

#endif
