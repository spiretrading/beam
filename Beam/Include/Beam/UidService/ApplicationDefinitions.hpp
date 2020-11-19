#ifndef BEAM_UID_APPLICATION_DEFINITIONS_HPP
#define BEAM_UID_APPLICATION_DEFINITIONS_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam::UidService {

  /** Encapsulates a standard UidClient used in an application. */
  class ApplicationUidClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder =
        Services::AuthenticatedServiceProtocolClientBuilder<
        ServiceLocator::VirtualServiceLocatorClient,
        Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::LiveTimer>;

      /** Defines the standard UidClient used for applications. */
      using Client = UidClient<SessionBuilder>;

      /**
       * Constructs an ApplicationUidClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      template<typename ServiceLocatorClient>
      ApplicationUidClient(Ref<ServiceLocatorClient> serviceLocatorClient);

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

      ApplicationUidClient(const ApplicationUidClient&) = delete;
      ApplicationUidClient& operator =(const ApplicationUidClient&) = delete;
  };

  template<typename ServiceLocatorClient>
  ApplicationUidClient::ApplicationUidClient(
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

  inline ApplicationUidClient::Client& ApplicationUidClient::operator *() {
    return m_client;
  }

  inline const ApplicationUidClient::Client&
      ApplicationUidClient::operator *() const {
    return m_client;
  }

  inline ApplicationUidClient::Client* ApplicationUidClient::operator ->() {
    return &m_client;
  }

  inline const ApplicationUidClient::Client*
      ApplicationUidClient::operator ->() const {
    return &m_client;
  }

  inline ApplicationUidClient::Client* ApplicationUidClient::Get() {
    return &m_client;
  }

  inline const ApplicationUidClient::Client* ApplicationUidClient::Get() const {
    return &m_client;
  }
}

#endif
