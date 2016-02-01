#ifndef AVALON_WEBSERVLETCONTAINER_HPP
#define AVALON_WEBSERVLETCONTAINER_HPP
#include "Avalon/IOTests/MockServerConnection.hpp"
#include "Avalon/Services/ServiceProtocolServlet.hpp"
#include "Avalon/Services/ServiceProtocolServletContainer.hpp"
#include "Avalon/Threading/LockRelease.hpp"
#include "Avalon/Threading/TestTimer.hpp"
#include "Avalon/WebServices/HttpServer.hpp"
#include "Avalon/WebServices/HttpSessionHandler.hpp"
#include "Avalon/WebServices/WebServices.hpp"
#include "Avalon/WebServices/WebServiceSession.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class WebServletContainer
      \brief Exposes a ServiceProtocolServlet via a web service.
      \tparam ServletType The type of servlet to manage.
      \tparam ChannelType The type of Channel used by the <i>ServletType</i>.
   */
  template<typename ServletType, typename ChannelType>
  class WebServletContainer {
    public:

      //! Constructs a WebServletContainer.
      /*!
        \param serverConnection The server accepting HTTP requests.
        \param keepAliveFactory The type of Timer used for Connection
                                keep-alives.
        \param expiryTimerFactory The type of Timer used for session expiry.
        \param servlet The servlet to expose via HTTP.
        \param serializerFactory The type of Serializer used for outgoing
                                 Service Messages.
        \param deserializerFactory The type of Deserializer used for incoming
                                   Service Messages.
      */
      WebServletContainer(IO::ServerConnection* serverConnection,
        const boost::function<Threading::Timer* ()>& keepAliveFactory,
        const boost::function<Threading::Timer* ()>& expiryTimerFactory,
        const Initializer<ServletType>& servlet,
        const Serialization::SerializerFactory& serializerFactory,
        const Serialization::DeserializerFactory& deserializerFactory);

      ~WebServletContainer();

      //! Returns <code>true</code> iff the servlet is operational.
      bool IsOpen() const;

      //! Opens the servlet, serving its requests/responses.
      void Open();

      //! Closes the servlet, disconnecting all clients.
      void Close();

      //! Associates a Service with an HttpRequestPredicate.
      /*!
        \tparam Service The Service to associate.
        \param predicate The predicate to associate with the <i>Service</i>.
      */
      template<typename Service>
      void SetServicePredicate(const HttpRequestPredicate& predicate);

    private:
      mutable boost::mutex m_mutex;
      HttpServer m_server;
      HttpSessionHandler m_sessionHandler;
      IO::Tests::MockServerConnection* m_serverConnection;
      Services::ServiceProtocolServletContainer<ServletType, ChannelType>
        m_servletContainer;
      bool m_isOpen;
      bool m_isClosing;
      std::vector<WebServiceSession*> m_sessions;

      static WebServiceSession* SessionFactory(
        IO::Tests::MockServerConnection* serverConnection,
        Serialization::Serializer* serializer,
        Serialization::Deserializer* deserializer);
      void Shutdown(boost::unique_lock<boost::mutex>& lock);
      void OnNewSession(HttpSession* baseSession);
      void OnSessionClosed(HttpSession* baseSession);
      void OnHttpServerClosed();
      template<typename Service>
      void OnRequest(HttpSession* session, HttpServerRequest* request,
        HttpServerResponse* response);
  };

  template<typename ServletType, typename ChannelType>
  WebServletContainer<ServletType, ChannelType>::WebServletContainer(
      IO::ServerConnection* serverConnection,
      const boost::function<Threading::Timer* ()>& keepAliveFactory,
      const boost::function<Threading::Timer* ()>& expiryTimerFactory,
      const Initializer<ServletType>& servlet,
      const Serialization::SerializerFactory& serializerFactory,
      const Serialization::DeserializerFactory& deserializerFactory)
      : m_serverConnection(new IO::Tests::MockServerConnection()),
        m_servletContainer(m_serverConnection,
          boost::factory<Serialization::BinarySerializer*>(),
          boost::factory<Serialization::BinaryDeserializer*>(),
          boost::factory<Threading::TestTimer*>(), servlet),
        m_isOpen(false),
        m_isClosing(false) {
    m_server.Initialize(serverConnection, keepAliveFactory,
      boost::bind(&WebServletContainer::OnHttpServerClosed, this));
    boost::function<HttpSession* ()> sessionFactory = boost::bind(
      &WebServletContainer::SessionFactory, m_serverConnection,
      boost::bind(serializerFactory), boost::bind(deserializerFactory));
    m_sessionHandler.Initialize(sessionFactory, expiryTimerFactory,
      boost::bind(&WebServletContainer::OnNewSession, this, _1),
      boost::bind(&WebServletContainer::OnSessionClosed, this, _1));
  }

  template<typename ServletType, typename ChannelType>
  WebServletContainer<ServletType, ChannelType>::~WebServletContainer() {
    Close();
  }

  template<typename ServletType, typename ChannelType>
  bool WebServletContainer<ServletType, ChannelType>::IsOpen() const {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    return m_isOpen;
  }

  template<typename ServletType, typename ChannelType>
  void WebServletContainer<ServletType, ChannelType>::Open() {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    if(m_isOpen) {
      return;
    }
    m_servletContainer.Open();
    m_server.Open();
    m_isOpen = true;
  }

  template<typename ServletType, typename ChannelType>
  void WebServletContainer<ServletType, ChannelType>::Close() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
    if(!m_isOpen) {
      return;
    }
    Shutdown(lock);
  }

  template<typename ServletType, typename ChannelType>
  template<typename Service>
  void WebServletContainer<ServletType, ChannelType>::SetServicePredicate(
      const HttpRequestPredicate& predicate) {
    m_server.SetHandler(m_sessionHandler.GetSlot(predicate, boost::bind(
      &WebServletContainer::OnRequest<Service>, this, _1, _2, _3)));
  }

  template<typename ServletType, typename ChannelType>
  WebServiceSession* WebServletContainer<ServletType,
      ChannelType>::SessionFactory(
      IO::Tests::MockServerConnection* serverConnection,
      Serialization::Serializer* serializer,
      Serialization::Deserializer* deserializer) {
    return new WebServiceSession(serverConnection, serializer, deserializer);
  }

  template<typename ServletType, typename ChannelType>
  void WebServletContainer<ServletType, ChannelType>::Shutdown(
      boost::unique_lock<boost::mutex>& lock) {
    m_isClosing = true;
    m_servletContainer.Close();
    {
      Threading::LockRelease<boost::unique_lock<boost::mutex> > release(lock);
      m_server.Close();
    }
    while(!m_sessions.empty()) {
      WebServiceSession* session = m_sessions.front();
      {
        Threading::LockRelease<boost::unique_lock<boost::mutex> > release(lock);

        // TODO, session deletes prior to this call.
        session->Close();
      }
    }
    m_isClosing = false;
    m_isOpen = false;
  }

  template<typename ServletType, typename ChannelType>
  void WebServletContainer<ServletType, ChannelType>::OnNewSession(
      HttpSession* baseSession) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    WebServiceSession* session = static_cast<WebServiceSession*>(baseSession);
    m_sessions.push_back(session);
  }

  template<typename ServletType, typename ChannelType>
  void WebServletContainer<ServletType, ChannelType>::OnSessionClosed(
      HttpSession* baseSession) {
    boost::lock_guard<boost::mutex> lock(m_mutex);
    WebServiceSession* session = static_cast<WebServiceSession*>(baseSession);
    session->Shutdown();
    Remove(m_sessions, session);
    m_sessionHandler.Remove(session);
  }

  template<typename ServletType, typename ChannelType>
  void WebServletContainer<ServletType, ChannelType>::OnHttpServerClosed() {}

  template<typename ServletType, typename ChannelType>
  template<typename Service>
  void WebServletContainer<ServletType, ChannelType>::OnRequest(
      HttpSession* session, HttpServerRequest* request,
      HttpServerResponse* response) {
    static_cast<WebServiceSession*>(session)->HandleRequest<Service>(request,
      response);
  }
}
}

#endif // AVALON_WEBSERVLETCONTAINER_HPP
