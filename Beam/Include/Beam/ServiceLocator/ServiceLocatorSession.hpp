#ifndef BEAM_SERVICELOCATORSESSION_HPP
#define BEAM_SERVICELOCATORSESSION_HPP
#include <vector>
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/Utilities/Algorithm.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class ServiceLocatorSession
      \brief Stores session info for a ServiceLocatorServlet Channel.
   */
  class ServiceLocatorSession : public AuthenticatedSession {
    public:

      //! Constructs a ServiceLocatorSession.
      ServiceLocatorSession();

      //! Acquires a ServiceLocatorSession.
      /*!
        \param session The ServiceLocatorSession to acquire.
      */
      ServiceLocatorSession(ServiceLocatorSession&& session);

      //! Returns the session id.
      const std::string& GetSessionId() const;

      //! Sets the session id.
      void SetSessionId(const std::string& sessionId);

      //! Returns the services being subscribed to by this Channel.
      const std::vector<std::string>& GetServiceSubscriptions() const;

      //! Subscribes to a service.
      /*!
        \param serviceName The name of the service to subscribe to.
      */
      void SubscribeService(const std::string& serviceName);

      //! Unsubscribes from a service.
      /*!
        \param serviceName The name of the service to unsubscribe from.
      */
      void UnsubscribeService(const std::string& serviceName);

      //! Returns the ServiceEntries registered by this Channel.
      const std::vector<ServiceEntry>& GetRegisteredServices() const;

      //! Registers a service.
      /*!
        \param serviceEntry The ServiceEntry describing the registered service.
      */
      void RegisterService(const ServiceEntry& serviceEntry);

      //! Unregisters a service id.
      /*!
        \param serviceId The id of the service unregister.
      */
      void UnregisterService(int serviceId);

      //! Monitors a DirectoryEntry.
      /*!
        \param entry The DirectoryEntry to monitor.
      */
      void Monitor(const DirectoryEntry& entry);

      //! Stops monitoring a DirectoryEntry.
      /*!
        \param entry The DirectoryEntry to stop monitoring.
      */
      void Unmonitor(const DirectoryEntry& entry);

      //! Returns the list of all DirectoryEntries being monitored.
      const std::vector<DirectoryEntry>& GetMonitors() const;

    private:
      std::string m_sessionId;
      std::vector<ServiceEntry> m_registeredServices;
      std::vector<std::string> m_serviceSubscriptions;
      std::vector<DirectoryEntry> m_monitors;
  };

  inline ServiceLocatorSession::ServiceLocatorSession() {}

  inline ServiceLocatorSession::ServiceLocatorSession(
      ServiceLocatorSession&& session)
      : AuthenticatedSession(std::move(session)),
        m_sessionId(std::move(session.m_sessionId)),
        m_registeredServices(std::move(session.m_registeredServices)),
        m_serviceSubscriptions(std::move(session.m_serviceSubscriptions)) {}

  inline const std::string& ServiceLocatorSession::GetSessionId() const {
    return m_sessionId;
  }

  inline void ServiceLocatorSession::SetSessionId(
      const std::string& sessionId) {
    m_sessionId = sessionId;
  }

  inline const std::vector<std::string>&
      ServiceLocatorSession::GetServiceSubscriptions() const {
    return m_serviceSubscriptions;
  }

  inline void ServiceLocatorSession::SubscribeService(
      const std::string& serviceName) {
    m_serviceSubscriptions.push_back(serviceName);
  }

  inline void ServiceLocatorSession::UnsubscribeService(
      const std::string& serviceName) {
    RemoveAll(m_serviceSubscriptions, serviceName);
  }

  inline const std::vector<ServiceEntry>&
      ServiceLocatorSession::GetRegisteredServices() const {
    return m_registeredServices;
  }

  inline void ServiceLocatorSession::RegisterService(
      const ServiceEntry& serviceEntry) {
    m_registeredServices.push_back(serviceEntry);
  }

  inline void ServiceLocatorSession::UnregisterService(int serviceId) {
    for(auto i = m_registeredServices.begin();
        i != m_registeredServices.end(); ++i) {
      if(i->GetId() == serviceId) {
        m_registeredServices.erase(i);
        return;
      }
    }
  }

  inline void ServiceLocatorSession::Monitor(const DirectoryEntry& entry) {
    m_monitors.push_back(entry);
  }

  inline void ServiceLocatorSession::Unmonitor(const DirectoryEntry& entry) {
    RemoveAll(m_monitors, entry);
  }

  inline const std::vector<DirectoryEntry>&
      ServiceLocatorSession::GetMonitors() const {
    return m_monitors;
  }
}
}

#endif
