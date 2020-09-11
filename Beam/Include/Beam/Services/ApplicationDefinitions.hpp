#ifndef BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"

namespace Beam::Services {
namespace Details {
  using DefaultSessionBuilder =
    Services::AuthenticatedServiceProtocolClientBuilder<
    ServiceLocator::VirtualServiceLocatorClient,
    MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
    Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
    Threading::LiveTimer>;
}

  /** Encapsulates a service client used in an application. */
  template<template<typename> class C, typename N,
    typename B = Details::DefaultSessionBuilder>
  class ApplicationClient {
    public:
      using SessionBuilder = B;

      /** Defines the standard AccountsClient used for applications. */
      using Client = C<SessionBuilder>;

      /** Constructs an ApplicationClient. */
      ApplicationClient() = default;

      /**
       * Builds the session.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      template<typename ServiceLocatorClient>
      void BuildSession(Ref<ServiceLocatorClient> serviceLocatorClient);

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
      std::unique_ptr<ServiceLocator::VirtualServiceLocatorClient>
        m_serviceLocatorClient;

      ApplicationClient(const ApplicationClient&) = delete;
      ApplicationClient& operator =(const ApplicationClient&) = delete;
  };

  template<template<typename> class C, typename N, typename B>
  template<typename ServiceLocatorClient>
  void ApplicationClient<C, N, B>::BuildSession(
      Ref<ServiceLocatorClient> serviceLocatorClient) {
    if(m_client) {
      m_client->Close();
      m_client.reset();
      m_serviceLocatorClient.reset();
    }
    m_serviceLocatorClient = ServiceLocator::MakeVirtualServiceLocatorClient(
      serviceLocatorClient.Get());
    auto addresses = ServiceLocator::LocateServiceAddresses(
      *m_serviceLocatorClient, N::name);
    auto delay = false;
    auto sessionBuilder = SessionBuilder(Ref(*m_serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          auto delayTimer = Threading::LiveTimer(boost::posix_time::seconds(3));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<Network::TcpSocketChannel>(addresses);
      },
      [] {
        return std::make_unique<Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
    m_client.emplace(sessionBuilder);
  }

  template<template<typename> class C, typename N, typename B>
  typename ApplicationClient<C, N, B>::Client&
      ApplicationClient<C, N, B>::operator *() {
    return *m_client;
  }

  template<template<typename> class C, typename N, typename B>
  const typename ApplicationClient<C, N, B>::Client&
      ApplicationClient<C, N, B>::operator *() const {
    return *m_client;
  }

  template<template<typename> class C, typename N, typename B>
  typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>:: operator ->() {
    return m_client.get_ptr();
  }

  template<template<typename> class C, typename N, typename B>
  const typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>::operator ->() const {
    return m_client.get_ptr();
  }

  template<template<typename> class C, typename N, typename B>
  typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>::Get() {
    return m_client.get_ptr();
  }

  template<template<typename> class C, typename N, typename B>
  const typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>::Get() const {
    return m_client.get_ptr();
  }
}

#endif
