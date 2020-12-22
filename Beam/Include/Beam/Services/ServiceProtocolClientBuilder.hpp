#ifndef BEAM_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#define BEAM_SERVICE_PROTOCOL_CLIENT_BUILDER_HPP
#include <functional>
#include "Beam/Pointers/ConstPointerPolicy.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/NativePointerPolicy.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam::Services {

  /**
   * Builds a ServiceProtocolClient and establishes a session.
   * @param <P> The type of MessageProtocol used to send and receive messages.
   * @param <T> The type of Timer used for heartbeats.
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
       * @param channelBuilder Used to build new Channels.
       * @param timerBuilder Used to build heartbeat Timers.
       */
      ServiceProtocolClientBuilder(ChannelBuilder channelBuilder,
        TimerBuilder timerBuilder);

      std::unique_ptr<Client> MakeClient(const ServiceSlots<Client>& slots);

      std::unique_ptr<Timer> MakeTimer();

    private:
      ChannelBuilder m_channelBuilder;
      TimerBuilder m_timerBuilder;
  };

  template<typename P, typename T>
  ServiceProtocolClientBuilder<P, T>::ServiceProtocolClientBuilder(
    ChannelBuilder channelBuilder, TimerBuilder timerBuilder)
    : m_channelBuilder(std::move(channelBuilder)),
      m_timerBuilder(std::move(timerBuilder)) {}

  template<typename P, typename T>
  std::unique_ptr<typename ServiceProtocolClientBuilder<P, T>::Client>
      ServiceProtocolClientBuilder<P, T>::MakeClient(
        const ServiceSlots<Client>& slots) {
    return std::make_unique<Client>(m_channelBuilder(), &slots, MakeTimer());
  }

  template<typename P, typename T>
  std::unique_ptr<typename ServiceProtocolClientBuilder<P, T>::Timer>
      ServiceProtocolClientBuilder<P, T>::MakeTimer() {
    return m_timerBuilder();
  }
}

#endif
