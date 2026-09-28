#ifndef BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICES_APPLICATION_DEFINITIONS_HPP
#include <concepts>
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
#include "Beam/Utilities/ApplicationInterrupt.hpp"

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

  /**
   * Retries connection failures using a custom wait function.
   * @param factory The factory invoked as an lvalue on each attempt.
   * @param wait Called with the retry delay; throw to stop retrying.
   * @return The connected client.
   */
  template<typename F, typename W> requires
    std::invocable<F&> && std::invocable<W&, boost::posix_time::time_duration>
  auto connect(F factory, W wait) {
    auto delay = boost::posix_time::seconds(1);
    while(true) {
      try {
        return factory();
      } catch(const AuthenticationException&) {
        throw;
      } catch(const ConnectException&) {
        wait(boost::posix_time::time_duration(delay));
        if(delay < boost::posix_time::seconds(30)) {
          delay += boost::posix_time::seconds(1);
        }
      }
    }
  }

  /**
   * Retries connection failures until connected or shutdown is requested.
   * @param factory The factory invoked as an lvalue on each attempt.
   * @return The connected client.
   * @throws std::runtime_error If shutdown is requested between attempts.
   */
  template<typename F> requires std::invocable<F&>
  auto connect(F factory) {
    return connect([&] {
      if(received_kill_event()) {
        throw std::runtime_error("");
      }
      return factory();
    }, [] (const auto& delay) {
      for(auto i = 0;
          i < delay.total_seconds() && !received_kill_event(); ++i) {
        sleep_for(boost::posix_time::seconds(1));
      }
    });
  }

  /**
   * Constructs a client, retrying connection failures until shutdown is
   * requested.
   * @tparam C The type of client to construct.
   * @param args The constructor arguments, stored by value and reused on each
   *        attempt.
   * @return The connected client.
   * @throws std::runtime_error If shutdown is requested between attempts.
   */
  template<typename C, typename... Args> requires
    std::constructible_from<C, Args&...>
  C connect(Args... args) {
    return connect([&] {
      return C(args...);
    });
  }

  template<template<typename> class C, typename N, typename B>
  template<typename... T>
  ApplicationClient<C, N, B>::ApplicationClient(
    Ref<typename SessionBuilder::ServiceLocatorClient> service_locator_client,
    T&&... args)
    : C<B>(make_session_builder<SessionBuilder>(
        service_locator_client, N::name), std::forward<T>(args)...) {}
}

#endif
