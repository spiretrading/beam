#ifndef BEAM_REGISTRY_APPLICATION_DEFINITIONS_HPP
#define BEAM_REGISTRY_APPLICATION_DEFINITIONS_HPP
#include <optional>
#include <string>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/RegistryService/RegistryClient.hpp"
#include "Beam/RegistryService/RegistryService.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>

namespace Beam {
namespace RegistryService {
namespace Details {
  using RegistryClientSessionBuilder =
    Services::AuthenticatedServiceProtocolClientBuilder<
    ServiceLocator::ApplicationServiceLocatorClient::Client,
    Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
    Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
    Threading::LiveTimer>;
}

  /*! \class ApplicationRegistryClient
      \brief Encapsulates a standard RegistryClient used in an application.
   */
  class ApplicationRegistryClient : private boost::noncopyable {
    public:

      //! Defines the standard RegistryClient used for applications.
      using Client = RegistryClient<Details::RegistryClientSessionBuilder>;

      //! Constructs an ApplicationRegistryClient.
      ApplicationRegistryClient() = default;

      //! Builds the session.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate sessions.
        \param socketThreadPool The SocketThreadPool used for the socket
               connection.
        \param timerThreadPool The TimerThreadPool used for heartbeats.
      */
      void BuildSession(Ref<ServiceLocator::
        ApplicationServiceLocatorClient::Client> serviceLocatorClient,
        Ref<Network::SocketThreadPool> socketThreadPool,
        Ref<Threading::TimerThreadPool> timerThreadPool);

      //! Returns a reference to the Client.
      Client& operator *();

      //! Returns a reference to the Client.
      const Client& operator *() const;

      //! Returns a pointer to the Client.
      Client* operator ->();

      //! Returns a pointer to the Client.
      const Client* operator ->() const;

      //! Returns a pointer to the Client.
      Client* Get();

      //! Returns a pointer to the Client.
      const Client* Get() const;

    private:
      std::optional<Client> m_client;
  };

  inline void ApplicationRegistryClient::BuildSession(
      Ref<ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient, Ref<Network::SocketThreadPool> socketThreadPool,
      Ref<Threading::TimerThreadPool> timerThreadPool) {
    if(m_client.has_value()) {
      m_client->Close();
      m_client = std::nullopt;
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto socketThreadPoolHandle = socketThreadPool.Get();
    auto timerThreadPoolHandle = timerThreadPool.Get();
    auto addresses = ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, SERVICE_NAME);
    auto delay = false;
    auto sessionBuilder = Details::RegistryClientSessionBuilder(
      Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          auto delayTimer = Threading::LiveTimer(boost::posix_time::seconds(3),
            Ref(*timerThreadPoolHandle));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<Network::TcpSocketChannel>(addresses,
          Ref(*socketThreadPoolHandle));
      },
      [=] {
        return std::make_unique<Threading::LiveTimer>(
          boost::posix_time::seconds(10), Ref(*timerThreadPoolHandle));
      });
    m_client.emplace(sessionBuilder);
  }

  inline ApplicationRegistryClient::Client&
      ApplicationRegistryClient::operator *() {
    return *m_client;
  }

  inline const ApplicationRegistryClient::Client&
      ApplicationRegistryClient::operator *() const {
    return *m_client;
  }

  inline ApplicationRegistryClient::Client*
      ApplicationRegistryClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationRegistryClient::Client*
      ApplicationRegistryClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationRegistryClient::Client* ApplicationRegistryClient::Get() {
    return &*m_client;
  }

  inline const ApplicationRegistryClient::Client*
      ApplicationRegistryClient::Get() const {
    return &*m_client;
  }
}
}

#endif
