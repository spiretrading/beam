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

  /*! \class SessionStoreConfig
      \brief Stores config needed to manage sessions.
   */
  struct SessionStoreConfig {

    //! Returns the default name used for the session cookie.
    static const std::string& GetDefaultSessionName();

    //! The name of the session Cookie.
    std::string m_sessionName;

    //! The domain this session is used in.
    std::string m_domain;

    //! The path the session is valid in.
    std::string m_path;

    //! Constructs a SessionStoreConfig with default values.
    SessionStoreConfig();
  };

  /*! \class SessionStore
      \brief Stores and manages HTTP sessions.
      \tparam SessionType The type of session to use.
   */
  template<typename SessionType>
  class SessionStore : private boost::noncopyable {
    public:

      //! The type of session to use.
      using Session = SessionType;

      //! Constructs a SessionStore with default values.
      SessionStore() = default;

      //! Constructs a SessionStore.
      /*!
        \param config The config to use to manage sessions.
      */
      SessionStore(SessionStoreConfig config);

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

      //! Ends a Session.
      /*!
        \param session The Session to end.
      */
      void End(Session& session);

      //! Sets an HTTP response's session cookie.
      /*!
        \param session The Session to encode.
        \param response Stores the cookie.
      */
      void SetSessionIdCookie(const Session& session,
        Out<HttpServerResponse> response) const;

    private:
      SessionStoreConfig m_config;
      std::string m_cookieAttributes;
      SynchronizedUnorderedSet<std::string> m_sessionIds;
      SynchronizedUnorderedMap<std::string, std::shared_ptr<Session>>
        m_sessions;
  };

  inline const std::string& SessionStoreConfig::GetDefaultSessionName() {
    static const std::string value = "sessionid";
    return value;
  }

  inline SessionStoreConfig::SessionStoreConfig()
      : m_sessionName{GetDefaultSessionName()},
        m_path{"/"} {}

  template<typename SessionType>
  SessionStore<SessionType>::SessionStore(SessionStoreConfig config)
      : m_config{std::move(config)} {
    if(!m_config.m_domain.empty()) {
      m_cookieAttributes += "; Domain=" + m_config.m_domain;
    }
    if(!m_config.m_path.empty()) {
      m_cookieAttributes += "; Path=" + m_config.m_path;
    }
  }

  template<typename SessionType>
  std::shared_ptr<typename SessionStore<SessionType>::Session>
      SessionStore<SessionType>::Get(const HttpServerRequest& request,
      Out<HttpServerResponse> response) {
    auto sessionCookie = request.GetCookie(m_config.m_sessionName);
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
    auto sessionCookie = request.GetCookie(m_config.m_sessionName);
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
  void SessionStore<SessionType>::End(Session& session) {
    session.SetExpired();
    m_sessions.Erase(session.GetId());
    m_sessionIds.Erase(session.GetId());
  }

  template<typename SessionType>
  void SessionStore<SessionType>::SetSessionIdCookie(const Session& session,
      Out<HttpServerResponse> response) const {
    response->SetCookie({m_config.m_sessionName,
      session.GetId() + m_cookieAttributes});
  }
}
}

#endif
