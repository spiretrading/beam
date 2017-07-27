#ifndef BEAM_SERVICES_APPLICATIONDEFINITIONS_HPP
#define BEAM_SERVICES_APPLICATIONDEFINITIONS_HPP
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"

namespace Beam {
namespace Services {
namespace Details {
  using DefaultSessionBuilder =
    Services::AuthenticatedServiceProtocolClientBuilder<
    ServiceLocator::ApplicationServiceLocatorClient::Client,
    MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
    Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
    Threading::LiveTimer>;
}

  /*! \class ApplicationClient
      \brief Encapsulates a service client used in an application.
   */
  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType = Details::DefaultSessionBuilder>
  class ApplicationClient : private boost::noncopyable {
    public:
      using SessionBuilder = SessionBuilderType;

      //! Defines the standard AccountsClient used for applications.
      using Client = ClientType<SessionBuilder>;

      //! Constructs an ApplicationClient.
      ApplicationClient() = default;

      //! Builds the session.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate sessions.
        \param socketThreadPool The SocketThreadPool used for the socket
               connection.
        \param timerThreadPool The TimerThreadPool used for heartbeats.
      */
      void BuildSession(
        RefType<ServiceLocator::ApplicationServiceLocatorClient::Client>
        serviceLocatorClient, RefType<Network::SocketThreadPool>
        socketThreadPool, RefType<Threading::TimerThreadPool> timerThreadPool);

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
      boost::optional<Client> m_client;
  };

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  void ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      BuildSession(RefType<ServiceLocator::ApplicationServiceLocatorClient::
      Client> serviceLocatorClient, RefType<Network::SocketThreadPool>
      socketThreadPool, RefType<Threading::TimerThreadPool> timerThreadPool) {
    if(m_client.is_initialized()) {
      m_client->Close();
      m_client.reset();
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto socketThreadPoolHandle = socketThreadPool.Get();
    auto timerThreadPoolHandle = timerThreadPool.Get();
    auto addresses = ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, ServiceName::name);
    auto delay = false;
    SessionBuilder sessionBuilder{Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          Threading::LiveTimer delayTimer{boost::posix_time::seconds(3),
            Ref(*timerThreadPoolHandle)};
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
      }};
    m_client.emplace(sessionBuilder);
  }

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  typename ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      Client& ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      operator *() {
    return *m_client;
  }

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  const typename ApplicationClient<ClientType, ServiceName,
      SessionBuilderType>::Client& ApplicationClient<ClientType, ServiceName,
      SessionBuilderType>::operator *() const {
    return *m_client;
  }

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  typename ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      Client* ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      operator ->() {
    return m_client.get_ptr();
  }

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  const typename ApplicationClient<ClientType, ServiceName,
      SessionBuilderType>::Client* ApplicationClient<ClientType, ServiceName,
      SessionBuilderType>::operator ->() const {
    return m_client.get_ptr();
  }

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  typename ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      Client* ApplicationClient<ClientType, ServiceName, SessionBuilderType>::
      Get() {
    return m_client.get_ptr();
  }

  template<template<typename> class ClientType, typename ServiceName,
    typename SessionBuilderType>
  const typename ApplicationClient<ClientType, ServiceName,
      SessionBuilderType>::Client* ApplicationClient<ClientType, ServiceName,
      SessionBuilderType>::Get() const {
    return m_client.get_ptr();
  }
}
}

#endif
