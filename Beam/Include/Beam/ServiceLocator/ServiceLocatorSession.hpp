#ifndef BEAM_SERVICE_LOCATOR_SESSION_HPP
#define BEAM_SERVICE_LOCATOR_SESSION_HPP
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam {

  /** Stores session info for a ServiceLocatorServlet Channel. */
  class ServiceLocatorSession : public AuthenticatedSession {
    public:

      /** Constructs a ServiceLocatorSession. */
      ServiceLocatorSession() noexcept;

      /**
       * Acquires a ServiceLocatorSession.
       * @param session The ServiceLocatorSession to acquire.
       */
      ServiceLocatorSession(ServiceLocatorSession&& session) noexcept;

      /** Returns the session id. */
      std::string get_session_id() const;

      /**
       * Sets the session id.
       * @param account The account that the session belongs to.
       * @param session_id The session id.
       */
      void set_session_id(
        const DirectoryEntry& account, const std::string& session_id);

      /**
       * Begins a login attempt.
       * @return <code>true</code> iff the session is in a state where a login
       *         attempt is permissible.
       */
      bool try_login();

      /** Marks the session as not logged in. */
      void reset_login();

      /** Returns the services being subscribed to by this Channel. */
      std::vector<std::string> get_service_subscriptions() const;

      /**
       * Subscribes to a service.
       * @param service_name The name of the service to subscribe to.
       */
      void subscribe_service(const std::string& service_name);

      /**
       * Unsubscribes from a service.
       * @param service_name The name of the service to unsubscribe from.
       */
      void unsubscribe_service(const std::string& service_name);

      /** Returns the ServiceEntries registered by this Channel. */
      std::vector<ServiceEntry> get_registered_services() const;

      /**
       * Registers a service.
       * @param service_entry The ServiceEntry describing the registered
       *        service.
       */
      void register_service(const ServiceEntry& service_entry);

      /**
       * Unregisters a service id.
       * @param service_id The id of the service unregister.
       */
      void unregister_service(int service_id);

      /**
       * Monitors a DirectoryEntry.
       * @param entry The DirectoryEntry to monitor.
       */
      void monitor(const DirectoryEntry& entry);

      /**
       * Stops monitoring a DirectoryEntry.
       * @param entry The DirectoryEntry to stop monitoring.
       */
      void unmonitor(const DirectoryEntry& entry);

      /** Returns the list of all DirectoryEntries being monitored. */
      std::vector<DirectoryEntry> get_monitors() const;

    private:
      enum class LoginState {
        NOT_LOGGED_IN,
        LOGGING_IN,
        LOGGED_IN
      };
      mutable boost::mutex m_mutex;
      LoginState m_login_state;
      std::string m_session_id;
      std::vector<ServiceEntry> m_registered_services;
      std::vector<std::string> m_service_subscriptions;
      std::vector<DirectoryEntry> m_monitors;
  };

  inline ServiceLocatorSession::ServiceLocatorSession() noexcept
    : m_login_state(LoginState::NOT_LOGGED_IN) {}

  inline ServiceLocatorSession::ServiceLocatorSession(
      ServiceLocatorSession&& session) noexcept
      : AuthenticatedSession(std::move(session)) {
    auto lock = boost::lock_guard(session.m_mutex);
    m_login_state = std::move(session.m_login_state);
    m_session_id = std::move(session.m_session_id);
    m_registered_services = std::move(session.m_registered_services);
    m_service_subscriptions = std::move(session.m_service_subscriptions);
    m_monitors = std::move(session.m_monitors);
  }

  inline std::string ServiceLocatorSession::get_session_id() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_session_id;
  }

  inline void ServiceLocatorSession::set_session_id(
      const DirectoryEntry& account, const std::string& session_id) {
    auto lock = boost::lock_guard(m_mutex);
    set_account(account);
    m_session_id = session_id;
    m_login_state = LoginState::LOGGED_IN;
  }

  inline bool ServiceLocatorSession::try_login() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_login_state == LoginState::NOT_LOGGED_IN) {
      m_login_state = LoginState::LOGGING_IN;
      return true;
    }
    return false;
  }

  inline void ServiceLocatorSession::reset_login() {
    auto lock = boost::lock_guard(m_mutex);
    reset_account();
    m_session_id.clear();
    m_login_state = LoginState::NOT_LOGGED_IN;
  }

  inline std::vector<std::string>
      ServiceLocatorSession::get_service_subscriptions() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_service_subscriptions;
  }

  inline void ServiceLocatorSession::subscribe_service(
      const std::string& service_name) {
    auto lock = boost::lock_guard(m_mutex);
    m_service_subscriptions.push_back(service_name);
  }

  inline void ServiceLocatorSession::unsubscribe_service(
      const std::string& service_name) {
    auto lock = boost::lock_guard(m_mutex);
    std::erase(m_service_subscriptions, service_name);
  }

  inline std::vector<ServiceEntry>
      ServiceLocatorSession::get_registered_services() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_registered_services;
  }

  inline void ServiceLocatorSession::register_service(
      const ServiceEntry& service_entry) {
    auto lock = boost::lock_guard(m_mutex);
    m_registered_services.push_back(service_entry);
  }

  inline void ServiceLocatorSession::unregister_service(int service_id) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = std::find_if(m_registered_services.begin(),
      m_registered_services.end(), [&] (const auto& service) {
        return service.get_id() == service_id;
      });
    if(i != m_registered_services.end()) {
      m_registered_services.erase(i);
    }
  }

  inline void ServiceLocatorSession::monitor(const DirectoryEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    m_monitors.push_back(entry);
  }

  inline void ServiceLocatorSession::unmonitor(const DirectoryEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    std::erase(m_monitors, entry);
  }

  inline std::vector<DirectoryEntry>
      ServiceLocatorSession::get_monitors() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_monitors;
  }
}

#endif
