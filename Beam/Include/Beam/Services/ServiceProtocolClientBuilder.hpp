#ifndef BEAM_SERVICEPROTOCOLCLIENTBUILDER_HPP
#define BEAM_SERVICEPROTOCOLCLIENTBUILDER_HPP
#include <functional>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/NativePointerPolicy.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {

  /*! \class ServiceProtocolClientBuilder
      \brief Builds a ServiceProtocolClient and establishes a session.
      \tparam MessageProtocolType The type of MessageProtocol used to send and
              receive messages.
      \tparam TimerType The type of Timer used for heartbeats.
   */
  template<typename MessageProtocolType, typename TimerType>
  class ServiceProtocolClientBuilder {
    public:

      //! The type of MessageProtocol used to send and receive messages.
      using MessageProtocol = MessageProtocolType;

      //! The type of Timer used for heartbeats.
      using Timer = TimerType;

      //! The type of ServiceProtocol used.
      using Client = ServiceProtocolClient<MessageProtocol,
        std::unique_ptr<Timer>, NativePointerPolicy>;

      //! The type of Channel used by the MessageProtocol.
      using Channel = typename MessageProtocol::Channel;

      //! Specifies the function used to build the Channel.
      using ChannelBuilder = std::function<std::unique_ptr<Channel> ()>;

      //! Specifies the function used to build the heartbeat Timer.
      using TimerBuilder = std::function<std::unique_ptr<Timer> ()>;

      //! Constructs a ServiceProtocolClientBuilder.
      /*!
        \param channelBuilder Used to build new Channels.
        \param timerBuilder Used to build heartbeat Timers.
      */
      ServiceProtocolClientBuilder(const ChannelBuilder& channelBuilder,
        const TimerBuilder& timerBuilder);

      std::unique_ptr<Client> Build(ServiceSlots<Client>& slots);

      void Open(Client& client);

    private:
      ChannelBuilder m_channelBuilder;
      TimerBuilder m_timerBuilder;
  };

  template<typename MessageProtocolType, typename TimerType>
  ServiceProtocolClientBuilder<MessageProtocolType, TimerType>::
      ServiceProtocolClientBuilder(const ChannelBuilder& channelBuilder,
      const TimerBuilder& timerBuilder)
      : m_channelBuilder(channelBuilder),
        m_timerBuilder(timerBuilder) {}

  template<typename MessageProtocolType, typename TimerType>
  std::unique_ptr<typename ServiceProtocolClientBuilder<
      MessageProtocolType, TimerType>::Client> ServiceProtocolClientBuilder<
      MessageProtocolType, TimerType>::Build(ServiceSlots<Client>& slots) {
    auto channel = m_channelBuilder();
    auto timer = m_timerBuilder();
    auto client = std::make_unique<Client>(std::move(channel), &slots,
      std::move(timer));
    return client;
  }

  template<typename MessageProtocolType, typename TimerType>
  void ServiceProtocolClientBuilder<MessageProtocolType, TimerType>::Open(
      Client& client) {}
}
}

#endif
