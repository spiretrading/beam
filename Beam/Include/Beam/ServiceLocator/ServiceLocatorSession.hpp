#ifndef BEAM_SERVICE_LOCATOR_SESSION_HPP
#define BEAM_SERVICE_LOCATOR_SESSION_HPP
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam::ServiceLocator {

  /** Stores session info for a ServiceLocatorServlet Channel. */
  class ServiceLocatorSession : public AuthenticatedSession {
    public:

      /** Constructs a ServiceLocatorSession. */
      ServiceLocatorSession();

      /**
       * Acquires a ServiceLocatorSession.
       * @param session The ServiceLocatorSession to acquire.
       */
      ServiceLocatorSession(ServiceLocatorSession&& session);

      /** Returns the session id. */
      std::string GetSessionId() const;

      /**
       * Sets the session id.
       * @param account The account that the session belongs to.
       * @param sessionId The session id.
       */
      void SetSessionId(const DirectoryEntry& account,
        const std::string& sessionId);

      /**
       * Begins a login attempt.
       * @return <code>true</code> iff the session is in a state where a login
       *         attempt is permissible.
       */
      bool TryLogin();

      /** Marks the session as not logged in. */
      void ResetLogin();

      /** Returns the services being subscribed to by this Channel. */
      std::vector<std::string> GetServiceSubscriptions() const;

      /**
       * Subscribes to a service.
       * @param serviceName The name of the service to subscribe to.
       */
      void SubscribeService(const std::string& serviceName);

      /**
       * Unsubscribes from a service.
       * @param serviceName The name of the service to unsubscribe from.
       */
      void UnsubscribeService(const std::string& serviceName);

      /** Returns the ServiceEntries registered by this Channel. */
      std::vector<ServiceEntry> GetRegisteredServices() const;

      /**
       * Registers a service.
       * @param serviceEntry The ServiceEntry describing the registered service.
       */
      void RegisterService(const ServiceEntry& serviceEntry);

      /**
       * Unregisters a service id.
       * @param serviceId The id of the service unregister.
       */
      void UnregisterService(int serviceId);

      /**
       * Monitors a DirectoryEntry.
       * @param entry The DirectoryEntry to monitor.
       */
      void Monitor(const DirectoryEntry& entry);

      /**
       * Stops monitoring a DirectoryEntry.
       * @param entry The DirectoryEntry to stop monitoring.
       */
      void Unmonitor(const DirectoryEntry& entry);

      /** Returns the list of all DirectoryEntries being monitored. */
      std::vector<DirectoryEntry> GetMonitors() const;

    private:
      enum class LoginState {
        NOT_LOGGED_IN,
        LOGGING_IN,
        LOGGED_IN
      };
      mutable boost::mutex m_mutex;
      LoginState m_loginState;
      std::string m_sessionId;
      std::vector<ServiceEntry> m_registeredServices;
      std::vector<std::string> m_serviceSubscriptions;
      std::vector<DirectoryEntry> m_monitors;
  };

  inline ServiceLocatorSession::ServiceLocatorSession()
    : m_loginState(LoginState::NOT_LOGGED_IN) {}

  inline ServiceLocatorSession::ServiceLocatorSession(
      ServiceLocatorSession&& session)
      : AuthenticatedSession(std::move(session)) {
    auto lock = boost::lock_guard(session.m_mutex);
    m_loginState = std::move(session.m_loginState);
    m_sessionId = std::move(session.m_sessionId);
    m_registeredServices = std::move(session.m_registeredServices);
    m_serviceSubscriptions = std::move(session.m_serviceSubscriptions);
  }

  inline std::string ServiceLocatorSession::GetSessionId() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_sessionId;
  }

  inline void ServiceLocatorSession::SetSessionId(
      const DirectoryEntry& account, const std::string& sessionId) {
    auto lock = boost::lock_guard(m_mutex);
    SetAccount(account);
    m_sessionId = sessionId;
    m_loginState = LoginState::LOGGED_IN;
  }

  inline bool ServiceLocatorSession::TryLogin() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_loginState == LoginState::NOT_LOGGED_IN) {
      m_loginState = LoginState::LOGGING_IN;
      return true;
    }
    return false;
  }

  inline void ServiceLocatorSession::ResetLogin() {
    auto lock = boost::lock_guard(m_mutex);
    ResetAccount();
    m_sessionId.clear();
    m_loginState = LoginState::NOT_LOGGED_IN;
  }

  inline std::vector<std::string>
      ServiceLocatorSession::GetServiceSubscriptions() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_serviceSubscriptions;
  }

  inline void ServiceLocatorSession::SubscribeService(
      const std::string& serviceName) {
    auto lock = boost::lock_guard(m_mutex);
    m_serviceSubscriptions.push_back(serviceName);
  }

  inline void ServiceLocatorSession::UnsubscribeService(
      const std::string& serviceName) {
    auto lock = boost::lock_guard(m_mutex);
    RemoveAll(m_serviceSubscriptions, serviceName);
  }

  inline std::vector<ServiceEntry>
      ServiceLocatorSession::GetRegisteredServices() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_registeredServices;
  }

  inline void ServiceLocatorSession::RegisterService(
      const ServiceEntry& serviceEntry) {
    auto lock = boost::lock_guard(m_mutex);
    m_registeredServices.push_back(serviceEntry);
  }

  inline void ServiceLocatorSession::UnregisterService(int serviceId) {
    auto lock = boost::lock_guard(m_mutex);
    auto i = std::find_if(m_registeredServices.begin(),
      m_registeredServices.end(),
      [&] (auto& service) {
        return service.GetId() == serviceId;
      });
    if(i != m_registeredServices.end()) {
      m_registeredServices.erase(i);
    }
  }

  inline void ServiceLocatorSession::Monitor(const DirectoryEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    m_monitors.push_back(entry);
  }

  inline void ServiceLocatorSession::Unmonitor(const DirectoryEntry& entry) {
    auto lock = boost::lock_guard(m_mutex);
    RemoveAll(m_monitors, entry);
  }

  inline std::vector<DirectoryEntry>
      ServiceLocatorSession::GetMonitors() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_monitors;
  }
}

#endif
