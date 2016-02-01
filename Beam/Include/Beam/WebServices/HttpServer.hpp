#ifndef AVALON_HTTPSERVER_HPP
#define AVALON_HTTPSERVER_HPP
#include "Avalon/IO/Channel.hpp"
#include "Avalon/IO/ConnectionServer.hpp"
#include "Avalon/Threading/Threading.hpp"
#include "Avalon/WebServices/HttpRequestSlot.hpp"
#include "Avalon/WebServices/WebServices.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpServer
      \brief Implements an HTTP server.
   */
  class HttpServer {
    public:

      //! Called when the ServerConnection is closed.
      typedef IO::ConnectionServer<
        HttpServerChannel>::ServerConnectionClosedSlot
        ServerConnectionClosedSlot;

      //! Constructs an HttpServer.
      HttpServer();

      ~HttpServer();

      //! Initializes the HttpServer.
      /*!
        \param serverConnection The ServerConnection accepting HTTP requests.
        \param keepAliveFactory Instantiates keep-alive Timers.
        \param serverConnectionClosedSlot The slot to call when the
                                          ServerConnection is closed.
      */
      void Initialize(IO::ServerConnection* serverConnection,
        const boost::function<Threading::Timer* ()>& keepAliveFactory,
        const ServerConnectionClosedSlot& serverConnectionClosedSlot);

      //! Returns <code>true</code> iff this server is open.
      bool IsOpen() const;

      //! Opens this server.
      void Open();

      //! Closes this server.
      void Close();

      //! Sets the handler for an HTTP request.
      /*!
        \param predicate The condition that the request must satisfy.
        \param slot The slot to call when the <i>predicate</i> is satisfied.
      */
      void SetHandler(const HttpRequestPredicate& predicate,
        const HttpRequestSlot::Slot& slot);

      //! Sets the blocking handler for an HTTP request.
      /*!
        \param slot The slot to handle the HTTP request.
      */
      void SetHandler(const HttpRequestSlot& slot);

    private:
      IO::ConnectionServer<HttpServerChannel> m_connectionServer;
      boost::function<Threading::Timer* ()> m_keepAliveFactory;
      std::vector<HttpRequestSlot> m_requestSlots;

      void Slot(const HttpRequestSlot::Slot& slot, HttpServerRequest* request,
        HttpServerResponse* response);
      void OnAccept(HttpServerChannel* channel);
      void OnChannelClosed(HttpServerChannel* channel);
  };
}
}

#endif // AVALON_HTTPSERVER_HPP
