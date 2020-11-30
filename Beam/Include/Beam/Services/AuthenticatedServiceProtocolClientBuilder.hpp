#ifndef BEAM_AUTHENTICATED_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#define BEAM_AUTHENTICATED_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"

namespace Beam::Services {

  /**
   * A ServiceProtocolClientBuilder that authenticates its session.
   * @param <C> The type of ServiceLocatorClient used to authenticate the
   *        session.
   * @param <P> The type of MessageProtocol used to send and receive messages.
   * @param <T> The type of Timer used for heartbeats.
   */
  template<typename C, typename P, typename T>
  class AuthenticatedServiceProtocolClientBuilder {
    public:

      /** The type of ServiceLocatorClient used to authenticate the session. */
      using ServiceLocatorClient = C;

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol =
        typename ServiceProtocolClientBuilder<P, T>::MessageProtocol;

      /** The type of Timer used for heartbeats. */
      using Timer = typename ServiceProtocolClientBuilder<P, T>::Timer;

      /** The type of ServiceProtocol used. */
      using Client = typename ServiceProtocolClientBuilder<P, T>::Client;

      /** The type of Channel used by the MessageProtocol. */
      using Channel = typename ServiceProtocolClientBuilder<P, T>::Channel;

      /** Specifies the function used to build the Channel. */
      using ChannelBuilder =
        typename ServiceProtocolClientBuilder<P, T>::ChannelBuilder;

      /** Specifies the function used to build the heartbeat Timer. */
      using TimerBuilder =
        typename ServiceProtocolClientBuilder<P, T>::TimerBuilder;

      /** The type of Authenticator used. */
      using Authenticator =
        typename ServiceLocator::Authenticator<Client>::type;

      /**
       * Constructs an AuthenticatedServiceProtocolClientBuilder.
       * @param serviceLocatorClient The ServiceLocatorClient used to
       *        authenticate the session.
       * @param channelBuilder Used to build new Channels.
       * @param timerBuilder Used to build heartbeat Timers.
       */
      template<typename CF>
      AuthenticatedServiceProtocolClientBuilder(CF&& serviceLocatorClient,
        ChannelBuilder channelBuilder, TimerBuilder timerBuilder);

      std::unique_ptr<Client> BuildClient(ServiceSlots<Client>& slots);

      std::unique_ptr<Timer> BuildTimer();

    private:
      GetOptionalLocalPtr<C> m_serviceLocatorClient;
      ServiceProtocolClientBuilder<P, T> m_clientBuilder;
  };

  template<typename C, typename P, typename T>
  template<typename CF>
  AuthenticatedServiceProtocolClientBuilder<C, P, T>::
    AuthenticatedServiceProtocolClientBuilder(CF&& serviceLocatorClient,
    ChannelBuilder channelBuilder, TimerBuilder timerBuilder)
    : m_serviceLocatorClient(std::forward<CF>(serviceLocatorClient)),
      m_clientBuilder(std::move(channelBuilder), std::move(timerBuilder)) {}

  template<typename C, typename P, typename T>
  std::unique_ptr<
      typename AuthenticatedServiceProtocolClientBuilder<C, P, T>::Client>
      AuthenticatedServiceProtocolClientBuilder<C, P, T>::BuildClient(
      ServiceSlots<Client>& slots) {
    auto client = m_clientBuilder.BuildClient(slots);
    ServiceLocator::Authenticate(ServiceLocator::SessionAuthenticator(
      Ref(*m_serviceLocatorClient)), *client);
    return client;
  }

  template<typename C, typename P, typename T>
  std::unique_ptr<
      typename AuthenticatedServiceProtocolClientBuilder<C, P, T>::Timer>
      AuthenticatedServiceProtocolClientBuilder<C, P, T>::BuildTimer() {
    return m_clientBuilder.BuildTimer();
  }
}

#endif
