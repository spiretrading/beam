#ifndef BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#include <string>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/TimeService/LiveTimer.hpp"

namespace Beam {

  /**
   * Wraps a constant string representing a service name.
   * @tparam N The name of the service to wrap.
   */
  template<const std::string& N>
  struct ServiceName {

    /** The name of the service. */
    inline static const std::string& name = N;
  };

  /** The default type of SessionBuilder used. */
  template<IsServiceLocatorClient C = ProtocolServiceLocatorClient<
    ServiceProtocolClientBuilder<
      MessageProtocol<std::unique_ptr<TcpSocketChannel>,
        BinarySender<SharedBuffer>>, LiveTimer>>>
  using DefaultSessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    C, MessageProtocol<std::unique_ptr<TcpSocketChannel>,
      BinarySender<SharedBuffer>, NullEncoder>, LiveTimer>;

  /** A SessionBuilder that uses ZLib compression. */
  template<IsServiceLocatorClient C = ProtocolServiceLocatorClient<
    ServiceProtocolClientBuilder<
      MessageProtocol<std::unique_ptr<TcpSocketChannel>,
        BinarySender<SharedBuffer>>, LiveTimer>>>
  using ZLibSessionBuilder = AuthenticatedServiceProtocolClientBuilder<
    C, MessageProtocol<std::unique_ptr<TcpSocketChannel>,
      BinarySender<SharedBuffer>, SizeDeclarativeEncoder<ZLibEncoder>>,
    LiveTimer>;

  /**
   * Returns a DefaultSessionBuilder.
   * @param client The ServiceLocatorClient used to authenticate sessions.
   */
  template<typename SessionBuilder, IsServiceLocatorClient C>
  auto make_session_builder(Ref<C>& client, const std::string& service) {
    return SessionBuilder(Ref(client),
      [=, client = client.get()] () mutable {
        return std::make_unique<TcpSocketChannel>(
          locate_service_addresses(*client, service));
      },
      [] {
        return std::make_unique<LiveTimer>(boost::posix_time::seconds(10));
      });
  }

  /**
   * Returns a DefaultSessionBuilder.
   * @param client The ServiceLocatorClient used to authenticate sessions.
   */
  template<IsServiceLocatorClient C>
  auto make_default_session_builder(
      Ref<C>& client, const std::string& service) {
    return make_session_builder<DefaultSessionBuilder<C>>(Ref(client), service);
  }

  /**
   * Encapsulates a standard application client.
   * @tparam C The type of client to encapsulate.
   * @tparam N The name of the service.
   * @tparam B The type of SessionBuilder to use.
   */
  template<template<typename> class C, typename N,
    typename B = DefaultSessionBuilder<>>
  class ApplicationClient : public C<B> {
    public:

      /** The type of SessionBuilder used. */
      using SessionBuilder = B;

      /** The type of client being encapsulated. */
      using Client = C<SessionBuilder>;

      /**
       * Constructs an ApplicationClient.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the session.
       * @param args The arguments to pass to the encapsulated client.
       */
      template<typename... T>
      explicit ApplicationClient(
        Ref<typename SessionBuilder::ServiceLocatorClient>
          service_locator_client, T&&... args);
  };

  template<template<typename> class C, typename N, typename B>
  template<typename... T>
  ApplicationClient<C, N, B>::ApplicationClient(
    Ref<typename SessionBuilder::ServiceLocatorClient> service_locator_client,
    T&&... args)
    : C<B>(make_session_builder<SessionBuilder>(
        service_locator_client, N::name), std::forward<T>(args)...) {}
}

#endif
