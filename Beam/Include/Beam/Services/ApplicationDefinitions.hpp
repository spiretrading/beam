#ifndef BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
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

      /**
       * Constructs an ApplicationClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      template<typename ServiceLocatorClient>
      ApplicationClient(Ref<ServiceLocatorClient> serviceLocatorClient);

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

      ApplicationClient(const ApplicationClient&) = delete;
      ApplicationClient& operator =(const ApplicationClient&) = delete;
  };

  template<template<typename> class C, typename N, typename B>
  template<typename ServiceLocatorClient>
  ApplicationClient<C, N, B>::ApplicationClient(
    Ref<ServiceLocatorClient> serviceLocatorClient)
    : m_serviceLocatorClient(ServiceLocator::MakeVirtualServiceLocatorClient(
        serviceLocatorClient.Get())),
      m_client(SessionBuilder(Ref(*m_serviceLocatorClient),
        [=] {
          return std::make_unique<Network::TcpSocketChannel>(
            ServiceLocator::LocateServiceAddresses(*m_serviceLocatorClient,
            N::name));
        },
        [] {
          return std::make_unique<Threading::LiveTimer>(
            boost::posix_time::seconds(10));
        })) {}

  template<template<typename> class C, typename N, typename B>
  typename ApplicationClient<C, N, B>::Client&
      ApplicationClient<C, N, B>::operator *() {
    return m_client;
  }

  template<template<typename> class C, typename N, typename B>
  const typename ApplicationClient<C, N, B>::Client&
      ApplicationClient<C, N, B>::operator *() const {
    return m_client;
  }

  template<template<typename> class C, typename N, typename B>
  typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>:: operator ->() {
    return &m_client;
  }

  template<template<typename> class C, typename N, typename B>
  const typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>::operator ->() const {
    return &m_client;
  }

  template<template<typename> class C, typename N, typename B>
  typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>::Get() {
    return &m_client;
  }

  template<template<typename> class C, typename N, typename B>
  const typename ApplicationClient<C, N, B>::Client*
      ApplicationClient<C, N, B>::Get() const {
    return &m_client;
  }
}

#endif
