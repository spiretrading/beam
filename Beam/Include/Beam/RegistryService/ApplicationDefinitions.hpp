#ifndef BEAM_REGISTRY_APPLICATION_DEFINITIONS_HPP
#define BEAM_REGISTRY_APPLICATION_DEFINITIONS_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/RegistryService/RegistryClient.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"

namespace Beam::RegistryService {

  /** Encapsulates a standard RegistryClient used in an application. */
  class ApplicationRegistryClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder =
        Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocator::VirtualServiceLocatorClient,
        Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::LiveTimer>;

      /** Defines the standard RegistryClient used for applications. */
      using Client = RegistryClient<SessionBuilder>;

      /**
       * Constructs an ApplicationRegistryClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      template<typename ServiceLocatorClient>
      ApplicationRegistryClient(Ref<ServiceLocatorClient> serviceLocatorClient);

      /** Returns a reference to the Client. */
      Client& operator *();

      /** Returns a reference to the Client. */
      const Client& operator *() const;

      /** Returns a pointer to the Client. */
      Client* operator ->();

      /** Returns a pointer to the Client. */
      const Client* operator ->() const;

      /** Returns a pointer to the Client. */
      Client* Get();

      /** Returns a pointer to the Client. */
      const Client* Get() const;

    private:
      std::unique_ptr<ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;
      Client m_client;

      ApplicationRegistryClient(const ApplicationRegistryClient&) = delete;
      ApplicationRegistryClient& operator =(
        const ApplicationRegistryClient&) = delete;
  };

  template<typename ServiceLocatorClient>
  ApplicationRegistryClient::ApplicationRegistryClient(
    Ref<ServiceLocatorClient> serviceLocatorClient)
    : m_serviceLocatorClient(ServiceLocator::MakeVirtualServiceLocatorClient(
        serviceLocatorClient.Get())),
      m_client(SessionBuilder(Ref(*m_serviceLocatorClient),
        [=] {
          return std::make_unique<Network::TcpSocketChannel>(
            ServiceLocator::LocateServiceAddresses(*m_serviceLocatorClient,
            SERVICE_NAME));
        },
        [] {
          return std::make_unique<Threading::LiveTimer>(
            boost::posix_time::seconds(10));
        })) {}

  inline ApplicationRegistryClient::Client&
      ApplicationRegistryClient::operator *() {
    return m_client;
  }

  inline const ApplicationRegistryClient::Client&
      ApplicationRegistryClient::operator *() const {
    return m_client;
  }

  inline ApplicationRegistryClient::Client*
      ApplicationRegistryClient::operator ->() {
    return &m_client;
  }

  inline const ApplicationRegistryClient::Client*
      ApplicationRegistryClient::operator ->() const {
    return &m_client;
  }

  inline ApplicationRegistryClient::Client* ApplicationRegistryClient::Get() {
    return &m_client;
  }

  inline const ApplicationRegistryClient::Client*
      ApplicationRegistryClient::Get() const {
    return &m_client;
  }
}

#endif
