#ifndef BEAM_WEBSOCKETCHANNEL_HPP
#define BEAM_WEBSOCKETCHANNEL_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/WebServices/WebServices.hpp"
#include "Beam/WebServices/WebSocket.hpp"
#include "Beam/WebServices/WebSocketConnection.hpp"
#include "Beam/WebServices/WebSocketReader.hpp"
#include "Beam/WebServices/WebSocketWriter.hpp"

namespace Beam {
namespace WebServices {

  /*! \class WebSocketChannel
      \brief Implements the Channel interface for a WebSocket.
   */
  template<typename WebSocketType>
  class WebSocketChannel : private boost::noncopyable {
    public:
      using WebSocket = WebSocketType;
      using Identifier = typename WebSocket::Channel::Identifier;
      using Connection = WebSocketConnection<WebSocket>;
      using Reader = WebSocketReader<WebSocket>;
      using Writer = WebSocketWriter<WebSocket>;

      //! Constructs a WebSocketChannel.
      /*!
        \param config The config used to initialize the WebSocket.
        \param channelBuilder Builds the Channel used to connect to the server.
      */
      WebSocketChannel(WebSocketConfig config,
        WebSocket::ChannelBuilder channelBuilder);

      //! Returns the underlying WebSocket.
      WebSocket& GetSocket();

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      std::shared_ptr<WebSocket> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;
  };

  template<typename WebSocketType>
  WebSocketChannel<WebSocketType>::WebSocketChannel(WebSocketConfig config,
      WebSocket::ChannelBuilder channelBuilder)
      : m_socket{std::make_shared<WebSocket>(std::move(config),
          std::move(channelBuilder)},
        m_connection{m_socket},
        m_reader{m_socket},
        m_writer{m_socket} {}

  template<typename WebSocketType>
  typename WebSocketChannel<WebSocketType>::WebSocket&
      WebSocketChannel<WebSocketType>::WebSocketChannel::GetSocket() {
    return *m_socket;
  }

  template<typename WebSocketType>
  const typename WebSocketChannel<WebSocketType>::Identifier&
      WebSocketChannel<WebSocketType>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename WebSocketType>
  typename WebSocketChannel<WebSocketType>::Connection&
      WebSocketChannel<WebSocketType>::GetConnection() {
    return m_connection;
  }

  template<typename WebSocketType>
  typename WebSocketChannel<WebSocketType>::Reader&
      WebSocketChannel<WebSocketType>::GetReader() {
    return m_reader;
  }

  template<typename WebSocketType>
  typename WebSocketChannel<WebSocketType>::Writer&
      WebSocketChannel<WebSocketType>::GetWriter() {
    return m_writer;
  }
}

  template<typename WebSocketType>
  struct ImplementsConcept<WebServices::WebSocketChannel<WebSocketType>,
      IO::Channel<
      typename WebServices::WebSocketChannel<WebSocketType>::Identifier,
      typename WebServices::WebSocketChannel<WebSocketType>::Connection,
      typename WebServices::WebSocketChannel<WebSocketType>::Reader,
      typename WebServices::WebSocketChannel<WebSocketType>::Writer>>
      : std::true_type {};
}

#endif
