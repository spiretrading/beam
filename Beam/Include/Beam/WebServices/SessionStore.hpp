#ifndef BEAM_WEBSERVICES_SESSIONSTORE_HPP
#define BEAM_WEBSERVICES_SESSIONSTORE_HPP
#include <memory>
#include <boost/noncopyable.hpp>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/NullSessionDataStore.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/SessionDataStore.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \struct SessionStoreConfig
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
      \tparam DataStoreType The type of data store used to persist sessions.
   */
  template<typename SessionType, typename DataStoreType = NullSessionDataStore>
  class SessionStore : private boost::noncopyable {
    public:

      //! The type of session to use.
      using Session = SessionType;

      //! The type of data store used to persist sessions.
      using DataStore = GetTryDereferenceType<DataStoreType>;

      //! Constructs a SessionStore with default values.
      SessionStore() = default;

      //! Constructs a SessionStore.
      /*!
        \param config The config to use to manage sessions.
      */
      SessionStore(SessionStoreConfig config);

      //! Constructs a SessionStore.
      /*!
        \param dataStore Initializes the DataStore.
      */
      template<typename DataStoreForward>
      SessionStore(DataStoreForward&& dataStore);

      //! Returns a Session associated with an HTTP request, creating it if it
      //! doesn't yet exist.
      /*!
        \param request The HTTP request containing the session id to find.
        \param response Sets the session id Cookie if the session doesn't exist.
        \return The Session associated with the <i>request</i>.
      */
      std::shared_ptr<Session> Get(const HttpRequest& request,
        Out<HttpResponse> response);

      //! Finds a Session associated with an HTTP request.
      /*!
        \param request The HTTP request containing the session id to find.
        \return The Session associated with the <i>request</i> or
                <code>nullptr</code> iff no Session exists.
      */
      std::shared_ptr<Session> Find(const HttpRequest& request) const;

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
        Out<HttpResponse> response) const;

      //! Persists a session permanently.
      /*!
        \param session The session to persist.
        \param response Stores the cookie.
      */
      void Persist(const Session& session, Out<HttpResponse> response);

      //! Makes a previously persistent session non persistent.
      /*!
        \param session The session to non persist.
        \param response Stores the cookie.
      */
      void NonPersist(const Session& session, Out<HttpResponse> response);

    private:
      SessionStoreConfig m_config;
      Beam::GetOptionalLocalPtr<DataStoreType> m_dataStore;
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

  template<typename SessionType, typename DataStoreType>
  SessionStore<SessionType, DataStoreType>::SessionStore(
      SessionStoreConfig config)
      : m_config{std::move(config)},
        m_dataStore{Initialize()} {}

  template<typename SessionType, typename DataStoreType>
  template<typename DataStoreForward>
  SessionStore<SessionType, DataStoreType>::SessionStore(
      DataStoreForward&& dataStore)
      : m_dataStore{std::forward<DataStoreForward>(dataStore)} {}

  template<typename SessionType, typename DataStoreType>
  std::shared_ptr<typename SessionStore<SessionType, DataStoreType>::Session>
      SessionStore<SessionType, DataStoreType>::Get(const HttpRequest& request,
      Out<HttpResponse> response) {
    auto sessionCookie = request.GetCookie(m_config.m_sessionName);
    if(!sessionCookie.is_initialized()) {
      auto session = Create();
      SetSessionIdCookie(*session, Store(response));
      return session;
    }
    auto session = m_sessions.FindValue(sessionCookie->GetValue());
    if(!session.is_initialized()) {
      auto persistentSession = m_dataStore->template Load<Session>(
        sessionCookie->GetValue());
      if(persistentSession == nullptr) {
        session = Create(); 
      } else {
        session = std::move(persistentSession);
      }
      SetSessionIdCookie(**session, Store(response));
      return *session;
    }
    return *session;
  }

  template<typename SessionType, typename DataStoreType>
  std::shared_ptr<typename SessionStore<SessionType, DataStoreType>::Session>
      SessionStore<SessionType, DataStoreType>::Find(
      const HttpRequest& request) const {
    auto sessionCookie = request.GetCookie(m_config.m_sessionName);
    if(!sessionCookie.is_initialized()) {
      return nullptr;
    }
    auto session = m_sessions.FindValue(sessionCookie->GetValue());
    if(!session.is_initialized()) {
      auto persistentSession = m_dataStore->template Load<Session>(
        sessionCookie->GetValue());
      if(persistentSession == nullptr) {
        return nullptr;
      } else {
        session = std::move(persistentSession);
      }
      return *session;
    }
    return *session;
  }

  template<typename SessionType, typename DataStoreType>
  std::shared_ptr<typename SessionStore<SessionType, DataStoreType>::Session>
      SessionStore<SessionType, DataStoreType>::Create() {
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

  template<typename SessionType, typename DataStoreType>
  void SessionStore<SessionType, DataStoreType>::End(Session& session) {
    m_dataStore->Delete(session);
    session.SetExpired();
    m_sessions.Erase(session.GetId());
    m_sessionIds.Erase(session.GetId());
  }

  template<typename SessionType, typename DataStoreType>
  void SessionStore<SessionType, DataStoreType>::SetSessionIdCookie(
      const Session& session, Out<HttpResponse> response) const {
    Cookie cookie{m_config.m_sessionName, session.GetId()};
    cookie.SetDomain(m_config.m_domain);
    cookie.SetPath(m_config.m_path);
    response->SetCookie(std::move(cookie));
  }

  template<typename SessionType, typename DataStoreType>
  void SessionStore<SessionType, DataStoreType>::Persist(
      const Session& session, Out<HttpResponse> response) {
    m_dataStore->Store(session);
    Cookie cookie{m_config.m_sessionName, session.GetId()};
    cookie.SetDomain(m_config.m_domain);
    cookie.SetPath(m_config.m_path);
    cookie.SetExpiration(boost::posix_time::second_clock::universal_time() +
      boost::posix_time::hours(24 * 365 * 10));
    response->SetCookie(std::move(cookie));
  }

  template<typename SessionType, typename DataStoreType>
  void SessionStore<SessionType, DataStoreType>::NonPersist(
      const Session& session, Out<HttpResponse> response) {
    m_dataStore->Delete(session);
    Cookie cookie{m_config.m_sessionName, session.GetId()};
    cookie.SetDomain(m_config.m_domain);
    cookie.SetPath(m_config.m_path);
    cookie.SetExpiration(boost::posix_time::not_a_date_time);
    response->SetCookie(std::move(cookie));
  }
}
}

#endif
