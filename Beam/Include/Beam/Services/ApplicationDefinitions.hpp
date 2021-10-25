#ifndef BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <Beam/Codecs/SizeDeclarativeDecoder.hpp>
#include <Beam/Codecs/SizeDeclarativeEncoder.hpp>
#include <Beam/Codecs/ZLibDecoder.hpp>
#include <Beam/Codecs/ZLibEncoder.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClientBox.hpp"
#include "Beam/Threading/LiveTimer.hpp"

namespace Beam::Services {

  /**
   * Wraps a constant string representing a service name.
   * @param <N> The name of the service to wrap.
   */
  template<const std::string& N>
  struct ServiceName {

    /** The name of the service. */
    inline static const std::string& name = N;
  };

  /** The default type of SessionBuilder used. */
  template<typename ServiceLocatorClient =
    Beam::ServiceLocator::ApplicationServiceLocatorClient::Client*>
  using DefaultSessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    ServiceLocatorClient, MessageProtocol<
      std::unique_ptr<Network::TcpSocketChannel>,
      Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
    Threading::LiveTimer>;

  /** A SessionBuilder that uses ZLib compression. */
  template<typename ServiceLocatorClient =
    Beam::ServiceLocator::ApplicationServiceLocatorClient::Client*>
  using ZLibSessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    ServiceLocatorClient, MessageProtocol<
      std::unique_ptr<Network::TcpSocketChannel>,
      Serialization::BinarySender<IO::SharedBuffer>,
      Codecs::SizeDeclarativeEncoder<Codecs::ZLibEncoder>>,
    Threading::LiveTimer>;

  /** Encapsulates a service client used in an application. */
  template<template<typename> class C, typename N,
    typename B = DefaultSessionBuilder<>>
  class ApplicationClient {
    public:
      using SessionBuilder = B;

      /** Defines the standard client used for applications. */
      using Client = C<SessionBuilder>;

      /**
       * Constructs an ApplicationClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       */
      explicit ApplicationClient(
        typename SessionBuilder::ServiceLocatorClient serviceLocatorClient);

      /**
       * Constructs an ApplicationClient.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate sessions.
       * @param args Any additional arguments to forward to the client.
       */
      template<typename... T>
      explicit ApplicationClient(
        typename SessionBuilder::ServiceLocatorClient serviceLocatorClient,
        T&&... args);

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
      Client m_client;

      ApplicationClient(const ApplicationClient&) = delete;
      ApplicationClient& operator =(const ApplicationClient&) = delete;
  };

  /**
   * Returns a DefaultSessionBuilder.
   * @param serviceLocatorClient The ServiceLocatorClient used to authenticate
   *        sessions.
   */
  template<typename SessionBuilder, typename ServiceLocatorClient>
  auto MakeSessionBuilder(ServiceLocatorClient&& serviceLocatorClient,
      const std::string& service) {
    auto clientBox = ServiceLocator::ServiceLocatorClientBox(&FullyDereference(
      serviceLocatorClient));
    return SessionBuilder(
      std::forward<ServiceLocatorClient>(serviceLocatorClient),
      [=] () mutable {
        return std::make_unique<Network::TcpSocketChannel>(
          ServiceLocator::LocateServiceAddresses(clientBox, service));
      },
      [] {
        return std::make_unique<Threading::LiveTimer>(
          boost::posix_time::seconds(10));
      });
  }

  /**
   * Returns a DefaultSessionBuilder.
   * @param serviceLocatorClient The ServiceLocatorClient used to authenticate
   *        sessions.
   */
  template<typename ServiceLocatorClient>
  auto MakeDefaultSessionBuilder(ServiceLocatorClient&& serviceLocatorClient,
      const std::string& service) {
    return MakeSessionBuilder<DefaultSessionBuilder<
      std::decay_t<ServiceLocatorClient>>>(std::forward<ServiceLocatorClient>(
        serviceLocatorClient), service);
  }

  template<template<typename> class C, typename N, typename B>
  ApplicationClient<C, N, B>::ApplicationClient(
    typename SessionBuilder::ServiceLocatorClient serviceLocatorClient)
    : m_client([&] {
        auto clientBox = ServiceLocator::ServiceLocatorClientBox(
          &FullyDereference(serviceLocatorClient));
        return SessionBuilder(std::move(serviceLocatorClient),
          [=] () mutable {
            return std::make_unique<Network::TcpSocketChannel>(
              ServiceLocator::LocateServiceAddresses(clientBox, N::name));
          },
          [] {
            return std::make_unique<Threading::LiveTimer>(
              boost::posix_time::seconds(10));
          });
      }()) {}

  template<template<typename> class C, typename N, typename B>
  template<typename... T>
  ApplicationClient<C, N, B>::ApplicationClient(
    typename SessionBuilder::ServiceLocatorClient serviceLocatorClient,
    T&&... args)
    : m_client([&] {
        auto clientBox = ServiceLocator::ServiceLocatorClientBox(
          &FullyDereference(serviceLocatorClient));
        return SessionBuilder(std::move(serviceLocatorClient),
          [=] () mutable {
            return std::make_unique<Network::TcpSocketChannel>(
              ServiceLocator::LocateServiceAddresses(clientBox, N::name));
          },
          [] {
            return std::make_unique<Threading::LiveTimer>(
              boost::posix_time::seconds(10));
          });
      }(), std::forward<T>(args)...) {}

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
