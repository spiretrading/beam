#ifndef BEAM_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#define BEAM_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#include <functional>
#include "Beam/Pointers/ConstPointerPolicy.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/NativePointerPolicy.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"

namespace Beam {

  /**
   * Builds a ServiceProtocolClient and establishes a session.
   * @tparam P The type of MessageProtocol used to send and receive messages.
   * @tparam T The type of Timer used for heartbeats.
   */
  template<typename P, typename T>
  class ServiceProtocolClientBuilder {
    public:

      /** The type of MessageProtocol used to send and receive messages. */
      using MessageProtocol = P;

      /** The type of Timer used for heartbeats. */
      using Timer = T;

      /** The type of ServiceProtocol used. */
      using Client = ServiceProtocolClient<MessageProtocol,
        std::unique_ptr<Timer>, ConstPointerPolicy<NativePointerPolicy>>;

      /** The type of Channel used by the MessageProtocol. */
      using Channel = typename MessageProtocol::Channel;

      /** Specifies the function used to build the Channel. */
      using ChannelBuilder = std::function<std::unique_ptr<Channel> ()>;

      /** Specifies the function used to build the heartbeat Timer. */
      using TimerBuilder = std::function<std::unique_ptr<Timer> ()>;

      /**
       * Constructs a ServiceProtocolClientBuilder.
       * @param channel_builder Used to build new Channels.
       * @param timer_builder Used to build heartbeat Timers.
       */
      ServiceProtocolClientBuilder(
        ChannelBuilder channel_builder, TimerBuilder timer_builder);

      /**
       * Constructs a ServiceProtocolClient.
       * @param slots The ServiceSlots the client will use.
       */
      std::unique_ptr<Client> make_client(const ServiceSlots<Client>& slots);

      /** Constructs a heartbeat Timer. */
      std::unique_ptr<Timer> make_timer();

    private:
      ChannelBuilder m_channel_builder;
      TimerBuilder m_timer_builder;
  };

  template<typename P, typename T>
  ServiceProtocolClientBuilder<P, T>::ServiceProtocolClientBuilder(
    ChannelBuilder channel_builder, TimerBuilder timer_builder)
    : m_channel_builder(std::move(channel_builder)),
      m_timer_builder(std::move(timer_builder)) {}

  template<typename P, typename T>
  std::unique_ptr<typename ServiceProtocolClientBuilder<P, T>::Client>
      ServiceProtocolClientBuilder<P, T>::make_client(
        const ServiceSlots<Client>& slots) {
    return std::make_unique<Client>(m_channel_builder(), &slots, make_timer());
  }

  template<typename P, typename T>
  std::unique_ptr<typename ServiceProtocolClientBuilder<P, T>::Timer>
      ServiceProtocolClientBuilder<P, T>::make_timer() {
    return m_timer_builder();
  }
}

#endif
