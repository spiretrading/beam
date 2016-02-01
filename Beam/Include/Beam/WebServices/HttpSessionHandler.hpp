#ifndef AVALON_HTTPSESSIONHANDLER_HPP
#define AVALON_HTTPSESSIONHANDLER_HPP
#include <boost/thread/mutex.hpp>
#include "Avalon/Threading/Threading.hpp"
#include "Avalon/WebServices/WebServices.hpp"
#include "Avalon/WebServices/HttpSession.hpp"
#include "Avalon/WebServices/HttpSessionRequestSlot.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class HttpSessionHandler
      \brief Creates and manages HTTP sessions.
   */
  class HttpSessionHandler {
    public:

      //! Constructs an HttpSessionHandler.
      HttpSessionHandler();

      ~HttpSessionHandler();

      //! Initializes this HttpSessionHandler.
      /*!
        \param sessionFactory Used to create new HTTP sessions.
        \param expiryTimerFactory Used to monitor a session's timeout.
        \param newSessionSlot The slot to call when a new session is created.
        \param sessionExpiredSlot The slot to call when a session expires.
      */
      void Initialize(const boost::function<HttpSession* ()>& sessionFactory,
        const boost::function<Threading::Timer* ()>& expiryTimerFactory,
        const HttpSession::NewSlot& newSessionSlot,
        const HttpSession::ExpiredSlot& sessionExpiredSlot);

      //! Returns a slot that's called when an HttpServer signals a request.
      /*!
        \param predicate The condition that the request must satisfy.
        \param slot The slot to call if the <i>predicate</i> is satisfied.
        \return A slot compatible with this HttpSessionHandler.
      */
      HttpRequestSlot GetSlot(const HttpRequestPredicate& predicate,
        const HttpSessionRequestSlot::Slot& slot);

      //! Returns a slot that's called when an HttpServer signals a request.
      /*!
        \param slot The slot to call upon the HttpServer request.
        \return A slot compatible with this HttpSessionHandler.
      */
      HttpRequestSlot GetSlot(const HttpSessionRequestSlot& slot);

      //! Removes an HttpSession.
      /*!
        \param session The HttpSession to remove.
      */
      void Remove(HttpSession* session);

    private:
      boost::mutex m_mutex;
      boost::function<HttpSession* ()> m_sessionFactory;
      boost::function<Threading::Timer* ()> m_expiryTimerFactory;
      std::map<std::string, HttpSession*> m_sessions;
      HttpSession::NewSlot m_newSessionSlot;
      HttpSession::ExpiredSlot m_sessionExpiredSlot;

      HttpSession* CreateSession();
      void OnRequest(HttpServerRequest* request, HttpServerResponse* response,
        const HttpSessionRequestSlot& slot);
  };
}
}

#endif // AVALON_HTTPSESSIONHANDLER_HPP
