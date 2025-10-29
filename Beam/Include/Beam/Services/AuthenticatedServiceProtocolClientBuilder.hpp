#ifndef BEAM_AUTHENTICATED_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#define BEAM_AUTHENTICATED_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#include <utility>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"

namespace Beam {

  /**
   * A ServiceProtocolClientBuilder that authenticates its session.
   * @tparam C The type of ServiceLocatorClient used to authenticate the
   *        session.
   * @tparam P The type of MessageProtocol used to send and receive messages.
   * @tparam T The type of Timer used for heartbeats.
   */
  template<IsServiceLocatorClient C, typename P, typename T> requires
    IsTimer<dereference_t<T>>
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

      /**
       * Constructs an AuthenticatedServiceProtocolClientBuilder.
       * @param service_locator_client The ServiceLocatorClient used to
       *        authenticate the session.
       * @param channel_builder Used to build new Channels.
       * @param timer_builder Used to build heartbeat Timers.
       */
      AuthenticatedServiceProtocolClientBuilder(
        Ref<ServiceLocatorClient> service_locator_client,
        ChannelBuilder channel_builder, TimerBuilder timer_builder);

      std::unique_ptr<Client> make_client(ServiceSlots<Client>& slots);
      std::unique_ptr<Timer> make_timer();

    private:
      ServiceLocatorClient* m_service_locator_client;
      ServiceProtocolClientBuilder<P, T> m_client_builder;
  };

  template<IsServiceLocatorClient C, typename P, typename T> requires
    IsTimer<dereference_t<T>>
  AuthenticatedServiceProtocolClientBuilder<C, P, T>::
    AuthenticatedServiceProtocolClientBuilder(
      Ref<ServiceLocatorClient> service_locator_client,
      ChannelBuilder channel_builder, TimerBuilder timer_builder)
    : m_service_locator_client(service_locator_client.get()),
      m_client_builder(std::move(channel_builder), std::move(timer_builder)) {}

  template<IsServiceLocatorClient C, typename P, typename T> requires
    IsTimer<dereference_t<T>>
  std::unique_ptr<
      typename AuthenticatedServiceProtocolClientBuilder<C, P, T>::Client>
        AuthenticatedServiceProtocolClientBuilder<C, P, T>::make_client(
          ServiceSlots<Client>& slots) {
    auto client = m_client_builder.make_client(slots);
    authenticate(SessionAuthenticator(Ref(*m_service_locator_client)), *client);
    return client;
  }

  template<IsServiceLocatorClient C, typename P, typename T> requires
    IsTimer<dereference_t<T>>
  std::unique_ptr<
      typename AuthenticatedServiceProtocolClientBuilder<C, P, T>::Timer>
        AuthenticatedServiceProtocolClientBuilder<C, P, T>::make_timer() {
    return m_client_builder.make_timer();
  }
}

#endif
