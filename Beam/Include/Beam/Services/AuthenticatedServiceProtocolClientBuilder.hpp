#ifndef BEAM_AUTHENTICATEDSERVICEPROTOCOLCLIENTBUILDER_HPP
#define BEAM_AUTHENTICATEDSERVICEPROTOCOLCLIENTBUILDER_HPP
#include <functional>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/NativePointerPolicy.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/ServiceLocator/SessionAuthenticator.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {

  /*! \class AuthenticatedServiceProtocolClientBuilder
      \brief A ServiceProtocolClientBuilder that authenticates its session.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient used to
              authenticate the session.
      \tparam MessageProtocolType The type of MessageProtocol used to send and
              receive messages.
      \tparam TimerType The type of Timer used for heartbeats.
   */
  template<typename ServiceLocatorClientType, typename MessageProtocolType,
    typename TimerType>
  class AuthenticatedServiceProtocolClientBuilder {
    public:

      //! The type of ServiceLocatorClient used to authenticate the session.
      using ServiceLocatorClient = ServiceLocatorClientType;

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

      //! The type of Authenticator used.
      using Authenticator =
        typename ServiceLocator::Authenticator<Client>::type;

      //! Constructs an AuthenticatedServiceProtocolClientBuilder.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate the session.
        \param channelBuilder Used to build new Channels.
        \param timerBuilder Used to build heartbeat Timers.
      */
      AuthenticatedServiceProtocolClientBuilder(Ref<ServiceLocatorClient>
        serviceLocatorClient, const ChannelBuilder& channelBuilder,
        const TimerBuilder& timerBuilder);

      std::unique_ptr<Client> Build(ServiceSlots<Client>& slots);

      void Open(Client& client);

    private:
      ServiceLocatorClient* m_serviceLocatorClient;
      ChannelBuilder m_channelBuilder;
      TimerBuilder m_timerBuilder;
  };

  template<typename ServiceLocatorClientType, typename MessageProtocolType,
    typename TimerType>
  AuthenticatedServiceProtocolClientBuilder<ServiceLocatorClientType,
      MessageProtocolType, TimerType>::
      AuthenticatedServiceProtocolClientBuilder(
      Ref<ServiceLocatorClient> serviceLocatorClient,
      const ChannelBuilder& channelBuilder, const TimerBuilder& timerBuilder)
      : m_serviceLocatorClient(serviceLocatorClient.Get()),
        m_channelBuilder(channelBuilder),
        m_timerBuilder(timerBuilder) {}

  template<typename ServiceLocatorClientType, typename MessageProtocolType,
    typename TimerType>
  std::unique_ptr<typename AuthenticatedServiceProtocolClientBuilder<
      ServiceLocatorClientType, MessageProtocolType, TimerType>::Client>
      AuthenticatedServiceProtocolClientBuilder<ServiceLocatorClientType,
      MessageProtocolType, TimerType>::Build(ServiceSlots<Client>& slots) {
    auto channel = m_channelBuilder();
    auto timer = m_timerBuilder();
    auto client = std::make_unique<Client>(std::move(channel), &slots,
      std::move(timer));
    return client;
  }

  template<typename ServiceLocatorClientType, typename MessageProtocolType,
    typename TimerType>
  void AuthenticatedServiceProtocolClientBuilder<ServiceLocatorClientType,
      MessageProtocolType, TimerType>::Open(Client& client) {
    ServiceLocator::OpenAndAuthenticate(
      ServiceLocator::SessionAuthenticator<ServiceLocatorClient>(
      Ref(*m_serviceLocatorClient)), client);
  }
}
}

#endif
