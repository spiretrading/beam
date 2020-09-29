#ifndef BEAM_UID_APPLICATION_DEFINITIONS_HPP
#define BEAM_UID_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
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
        ServiceLocator::ApplicationServiceLocatorClient::Client,
        Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::LiveTimer>;

      /** Defines the standard UidClient used for applications. */
      using Client = UidClient<SessionBuilder>;

      /** Constructs an ApplicationUidClient. */
      ApplicationUidClient() = default;

      /**
       * Builds the session.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      void BuildSession(
        Ref<ServiceLocator::ApplicationServiceLocatorClient::Client>
        serviceLocatorClient);

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
      boost::optional<Client> m_client;

      ApplicationUidClient(const ApplicationUidClient&) = delete;
      ApplicationUidClient& operator =(const ApplicationUidClient&) = delete;
  };

  inline void ApplicationUidClient::BuildSession(
      Ref<ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient) {
    m_client = boost::none;
    auto addresses = ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClient, SERVICE_NAME);
    auto sessionBuilder = SessionBuilder(Ref(serviceLocatorClient),
      [=] {
        return std::make_unique<Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(std::move(sessionBuilder));
  }

  inline ApplicationUidClient::Client& ApplicationUidClient::operator *() {
    return *m_client;
  }

  inline const ApplicationUidClient::Client&
      ApplicationUidClient::operator *() const {
    return *m_client;
  }

  inline ApplicationUidClient::Client* ApplicationUidClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationUidClient::Client*
      ApplicationUidClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationUidClient::Client* ApplicationUidClient::Get() {
    return &*m_client;
  }

  inline const ApplicationUidClient::Client* ApplicationUidClient::Get() const {
    return &*m_client;
  }
}

#endif
