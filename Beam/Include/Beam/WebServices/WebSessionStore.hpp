#ifndef BEAM_WEB_SESSION_STORE_HPP
#define BEAM_WEB_SESSION_STORE_HPP
#include <memory>
#include <string>
#include "Beam/Collections/SynchronizedMap.hpp"
#include "Beam/Collections/SynchronizedSet.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/ServiceLocator/SessionEncryption.hpp"
#include "Beam/WebServices/HttpRequest.hpp"
#include "Beam/WebServices/HttpResponse.hpp"
#include "Beam/WebServices/NullWebSessionDataStore.hpp"
#include "Beam/WebServices/WebSession.hpp"
#include "Beam/WebServices/WebSessionDataStore.hpp"

namespace Beam {

  /** Stores config needed to manage sessions. */
  struct WebSessionStoreConfig {

    /** The default name used for the session cookie. */
    static inline const std::string DEFAULT_WEB_SESSION_NAME = "sessionid";

    /** The name of the session Cookie. */
    std::string m_session_name;

    /** The domain this session is used in. */
    std::string m_domain;

    /** The path the session is valid in. */
    std::string m_path;

    /** Constructs a WebSessionStoreConfig with default values. */
    WebSessionStoreConfig();
  };

  /**
   * Stores and manages HTTP sessions.
   * @tparam S The type of session to use.
   * @tparam D The type of data store used to persist sessions.
   */
  template<std::derived_from<WebSession> S,
    typename D = NullWebSessionDataStore> requires
      IsWebSessionDataStore<dereference_t<D>>
  class WebSessionStore {
    public:

      /** The type of session to use. */
      using Session = S;

      /** The type of data store used to persist sessions. */
      using DataStore = dereference_t<D>;

      /** Constructs a WebSessionStore with default values. */
      WebSessionStore() = default;

      /**
       * Constructs a WebSessionStore.
       * @param config The config to use to manage sessions.
       */
      WebSessionStore(WebSessionStoreConfig config);

      /**
       * Constructs a WebSessionStore.
       * @param data_store Initializes the DataStore.
       */
      template<Initializes<D> DF>
      WebSessionStore(DF&& data_store);

      /**
       * Returns a WebSession associated with an HTTP request, creating it if it
       * doesn't yet exist.
       * @param request The HTTP request containing the session id to find.
       * @param response Sets the session id Cookie if the session doesn't
       *        exist.
       * @return The WebSession associated with the request.
       */
      std::shared_ptr<Session> get(
        const HttpRequest& request, Out<HttpResponse> response);

      /**
       * Finds a WebSession associated with an HTTP request.
       * @param request The HTTP request containing the session id to find.
       * @return The Session associated with the request or nullptr iff no
       *         Session exists.
       */
      std::shared_ptr<Session> find(const HttpRequest& request) const;

      /**
       * Creates a Session.
       * @return The new Session.
       */
      std::shared_ptr<Session> create();

      /**
       * Ends a Session.
       * @param session The Session to end.
       */
      void end(Session& session);

      /**
       * Sets an HTTP response's session cookie.
       * @param session The Session to encode.
       * @param response Stores the cookie.
       */
      void set_web_session_id_cookie(
        const Session& session, Out<HttpResponse> response) const;

      /**
       * Persists a session permanently.
       * @param session The session to persist.
       * @param response Stores the cookie.
       */
      void persist(const Session& session, Out<HttpResponse> response);

      /**
       * Makes a previously persistent session non persistent.
       * @param session The session to make non persistent.
       * @param response Stores the cookie.
       */
      void unpersist(const Session& session, Out<HttpResponse> response);

    private:
      WebSessionStoreConfig m_config;
      local_ptr_t<D> m_data_store;
      SynchronizedUnorderedSet<std::string> m_session_ids;
      SynchronizedUnorderedMap<std::string, std::shared_ptr<Session>>
        m_sessions;
  };

  inline WebSessionStoreConfig::WebSessionStoreConfig()
    : m_session_name(DEFAULT_WEB_SESSION_NAME),
      m_path("/") {}

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  WebSessionStore<S, D>::WebSessionStore(WebSessionStoreConfig config)
    : m_config(std::move(config)),
      m_data_store(init()) {}

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  template<Initializes<D> DF>
  WebSessionStore<S, D>::WebSessionStore(DF&& data_store)
    : m_data_store(std::forward<DF>(data_store)) {}

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  std::shared_ptr<typename WebSessionStore<S, D>::Session>
      WebSessionStore<S, D>::get(
        const HttpRequest& request, Out<HttpResponse> response) {
    auto session_cookie = request.get_cookie(m_config.m_session_name);
    if(!session_cookie) {
      auto session = create();
      set_web_session_id_cookie(*session, out(response));
      return session;
    } else if(auto session = m_sessions.find(session_cookie->get_value())) {
      return *session;
    }
    auto session = [&] {
      if(auto persistent_session = m_data_store->template load<Session>(
          session_cookie->get_value())) {
        return std::shared_ptr<Session>(std::move(persistent_session));
      } else {
        return create();
      }
    }();
    set_web_session_id_cookie(*session, out(response));
    return session;
  }

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  std::shared_ptr<typename WebSessionStore<S, D>::Session>
      WebSessionStore<S, D>::find(const HttpRequest& request) const {
    auto session_cookie = request.get_cookie(m_config.m_session_name);
    if(!session_cookie) {
      return nullptr;
    } else if(auto session = m_sessions.find(session_cookie->get_value())) {
      return *session;
    } else if(auto persistent_session = m_data_store->template load<Session>(
        session_cookie->get_value())) {
      return persistent_session;
    }
    return nullptr;
  }

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  std::shared_ptr<typename WebSessionStore<S, D>::Session>
      WebSessionStore<S, D>::create() {
    auto session_id = std::string();
    m_session_ids.with([&] (auto& session_ids) {
      do {
        session_id = generate_session_id();
      } while(session_ids.find(session_id) != session_ids.end());
      session_ids.insert(session_id);
    });
    auto session = std::make_shared<Session>(std::move(session_id));
    m_sessions.insert(session->get_id(), session);
    return session;
  }

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  void WebSessionStore<S, D>::end(Session& session) {
    m_data_store->remove(session);
    session.set_expired();
    m_sessions.erase(session.get_id());
    m_session_ids.erase(session.get_id());
  }

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  void WebSessionStore<S, D>::set_web_session_id_cookie(
      const Session& session, Out<HttpResponse> response) const {
    auto cookie = Cookie(m_config.m_session_name, session.get_id());
    cookie.set_domain(m_config.m_domain);
    cookie.set_path(m_config.m_path);
    response->set_cookie(std::move(cookie));
  }

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  void WebSessionStore<S, D>::persist(
      const Session& session, Out<HttpResponse> response) {
    m_data_store->store(session);
    auto cookie = Cookie(m_config.m_session_name, session.get_id());
    cookie.set_domain(m_config.m_domain);
    cookie.set_path(m_config.m_path);
    cookie.set_expiration(boost::posix_time::second_clock::universal_time() +
      boost::posix_time::hours(24 * 365 * 10));
    response->set_cookie(std::move(cookie));
  }

  template<std::derived_from<WebSession> S, typename D> requires
    IsWebSessionDataStore<dereference_t<D>>
  void WebSessionStore<S, D>::unpersist(
      const Session& session, Out<HttpResponse> response) {
    m_data_store->remove(session);
    auto cookie = Cookie(m_config.m_session_name, session.get_id());
    cookie.set_domain(m_config.m_domain);
    cookie.set_path(m_config.m_path);
    cookie.set_expiration(boost::posix_time::not_a_date_time);
    response->set_cookie(std::move(cookie));
  }
}

#endif
