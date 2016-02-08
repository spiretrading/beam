#ifndef BEAM_WEBSERVICES_SESSIONSTORE_HPP
#define BEAM_WEBSERVICES_SESSIONSTORE_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Out.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/Utilities/SynchronizedMap.hpp"
#include "Beam/Utilities/SynchronizedSet.hpp"
#include "Beam/WebServices/HttpServerRequest.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class SessionStore
      \brief Stores and manages HTTP sessions.
      \tparam SessionType The type of session to use.
   */
  template<typename SessionType>
  class SessionStore : private boost::noncopyable {
    public:

      //! The type of session to use.
      using Session = SessionType;

      //! Returns the default name used for the session cookie.
      static const std::string& GetDefaultSessionName();

      //! Constructs a SessionStore with default values.
      SessionStore();

      //! Constructs a SessionStore with a specified session name.
      SessionStore(std::string sessionName);

      //! Returns a Session associated with an HTTP request, creating it if it
      //! doesn't yet exist.
      /*!
        \param request The HTTP request containing the session id to find.
        \param response Sets the session id Cookie if the session doesn't exist.
        \return The Session associated with the <i>request</i>.
      */
      std::shared_ptr<Session> Get(const HttpServerRequest& request,
        Out<HttpServerResponse> response);

      //! Finds a Session associated with an HTTP request.
      /*!
        \param request The HTTP request containing the session id to find.
        \return The Session associated with the <i>request</i> or
                <code>nullptr</code> iff no Session exists.
      */
      std::shared_ptr<Session> Find(const HttpServerRequest& request) const;

      //! Creates a Session.
      /*!
        \return The new Session.
      */
      std::shared_ptr<Session> Create();

      //! Sets an HTTP response's session cookie.
      /*!
        \param session The Session to encode.
        \param response Stores the cookie.
      */
      void SetSessionIdCookie(const Session& session,
        Out<HttpServerResponse> response) const;

    private:
      std::string m_sessionName;
      SynchronizedUnorderedSet<std::string> m_sessionIds;
      SynchronizedUnorderedMap<std::string, std::shared_ptr<Session>>
        m_sessions;
  };

  template<typename SessionType>
  const std::string& SessionStore<SessionType>::GetDefaultSessionName() {
    static const std::string value = "sessionid";
    return value;
  }

  template<typename SessionType>
  SessionStore<SessionType>::SessionStore()
      : SessionStore{GetDefaultSessionName()} {}

  template<typename SessionType>
  SessionStore<SessionType>::SessionStore(std::string sessionName)
      : m_sessionName{std::move(sessionName)} {}

  template<typename SessionType>
  std::shared_ptr<typename SessionStore<SessionType>::Session>
      SessionStore<SessionType>::Get(const HttpServerRequest& request,
      Out<HttpServerResponse> response) {
    auto sessionCookie = request.GetCookie(m_sessionName);
    if(!sessionCookie.is_initialized()) {
      auto session = Create();
      SetSessionIdCookie(*session, Store(response));
      return session;
    }
    auto session = m_sessions.FindValue(sessionCookie->GetValue());
    if(!session.is_initialized()) {
      auto session = Create();
      SetSessionIdCookie(*session, Store(response));
      return session;
    }
    return *session;
  }

  template<typename SessionType>
  std::shared_ptr<typename SessionStore<SessionType>::Session>
      SessionStore<SessionType>::Find(const HttpServerRequest& request) const {
    auto sessionCookie = request.GetCookie(m_sessionName);
    if(!sessionCookie.is_initialized()) {
      return nullptr;
    }
    auto session = m_sessions.FindValue(sessionCookie->GetValue());
    if(!session.is_initialized()) {
      return nullptr;
    }
    return *session;
  }

  template<typename SessionType>
  std::shared_ptr<typename SessionStore<SessionType>::Session>
      SessionStore<SessionType>::Create() {
    std::string sessionId;
    m_sessionIds.With(
      [&] (auto& sessionIds) {
        do {
          sessionId = ServiceLocator::GenerateSessionId();
        } while(sessionIds.find(sessionId) != sessionIds.end());
        sessionIds.insert(sessionId);
      });
    auto session = std::make_shared<Session>(std::move(sessionId));
    m_sessions.Insert(session->GetId(), session);
    return session;
  }

  template<typename SessionType>
  void SessionStore<SessionType>::SetSessionIdCookie(const Session& session,
      Out<HttpServerResponse> response) const {
    response->SetCookie({m_sessionName, session.GetId() + "; path=/"});
  }
}
}

#endif
