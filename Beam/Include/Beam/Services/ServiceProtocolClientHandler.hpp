#ifndef BEAM_SERVICE_PROTOCOL_CLIENT_HANDLER_HPP
#define BEAM_SERVICE_PROTOCOL_CLIENT_HANDLER_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"

namespace Beam::Services {

  /**
   * Instantiates ServiceProtocolClients for a service.
   * @param <B> The type used to build ServiceProtocolClients.
   */
  template<typename B>
  class ServiceProtocolClientHandler {
    public:

      /** The type used to build ServiceProtocolClients. */
      using ServiceProtocolClientBuilder = B;

      /** The type of ServiceProtocolClient used. */
      using Client = typename
        ServiceProtocolClientHandler<B>::ServiceProtocolClientBuilder::Client;

      /**
       * The type of function used to handle a reconnection event.
       * @param client The client used for the reconnection.
       */
      using ReconnectHandler = std::function<
        void (const std::shared_ptr<Client>& client)>;

      /**
       * Constructs a ServiceProtocolClientHandler.
       * @param builder Initializes the builder used for ServiceProtocolClients.
       */
      template<typename BF>
      ServiceProtocolClientHandler(BF&& builder);

      /**
       * Constructs a ServiceProtocolClientHandler.
       * @param builder Initializes the builder used for ServiceProtocolClients.
       * @param reconnectHandler The function to call to handle a reconnection
       *        event;
       */
      template<typename BF>
      ServiceProtocolClientHandler(BF&& builder,
        ReconnectHandler reconnectHandler);

      ~ServiceProtocolClientHandler();

      /** Returns the slots used by the ServiceProtocolClients. */
      const ServiceSlots<Client>& GetSlots() const;

      /** Returns the slots used by the ServiceProtocolClients. */
      ServiceSlots<Client>& GetSlots();

      /** Returns the most recently instantiated client. */
      std::shared_ptr<Client> GetClient();

      void Close();

    private:
      Threading::RecursiveMutex m_mutex;
      GetOptionalLocalPtr<B> m_builder;
      ServiceSlots<Client> m_slots;
      ReconnectHandler m_reconnectHandler;
      std::shared_ptr<Client> m_client;
      Routines::RoutineHandler m_messageHandler;
      IO::OpenState m_openState;

      ServiceProtocolClientHandler(
        const ServiceProtocolClientHandler&) = delete;
      ServiceProtocolClientHandler& operator =(
        const ServiceProtocolClientHandler&) = delete;
      void MessageLoop();
  };

  template<typename B>
  template<typename BF>
  ServiceProtocolClientHandler<B>::ServiceProtocolClientHandler(BF&& builder)
    : ServiceProtocolClientHandler(std::forward<BF>(builder),
        [] (const std::shared_ptr<Client>&) {}) {}

  template<typename B>
  template<typename BF>
  ServiceProtocolClientHandler<B>::ServiceProtocolClientHandler(BF&& builder,
    ReconnectHandler reconnectHandler)
    : m_builder(std::forward<BF>(builder)),
      m_reconnectHandler(std::move(reconnectHandler)),
      m_client(m_builder->BuildClient(m_slots)),
      m_messageHandler(Routines::Spawn(
        std::bind(&ServiceProtocolClientHandler::MessageLoop, this))) {}

  template<typename B>
  ServiceProtocolClientHandler<B>::~ServiceProtocolClientHandler() {
    Close();
  }

  template<typename B>
  const ServiceSlots<typename ServiceProtocolClientHandler<B>::Client>&
      ServiceProtocolClientHandler<B>::GetSlots() const {
    return m_slots;
  }

  template<typename B>
  ServiceSlots<typename ServiceProtocolClientHandler<B>::Client>&
      ServiceProtocolClientHandler<B>::GetSlots() {
    return m_slots;
  }

  template<typename B>
  std::shared_ptr<typename ServiceProtocolClientHandler<B>::Client>
      ServiceProtocolClientHandler<B>::GetClient() {
    while(true) {
      auto lock = boost::unique_lock(m_mutex);
      if(m_client != nullptr) {
        return m_client;
      }
      m_openState.EnsureOpen();
      try {
        m_client = m_builder->BuildClient(m_slots);
        try {
          m_reconnectHandler(m_client);
        } catch(const std::exception&) {
          m_client = nullptr;
          m_openState.Close();
          BOOST_RETHROW;
        }
        return m_client;
      } catch(const IO::ConnectException&) {
        auto release = Threading::Release(lock);
        auto timer = m_builder->BuildTimer();
        timer->Start();
        timer->Wait();
      } catch(const std::exception&) {
        m_openState.Close();
        BOOST_RETHROW;
      }
    }
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto client = [&] {
      auto lock = boost::lock_guard(m_mutex);
      return std::exchange(m_client, nullptr);
    }();
    if(client) {
      client->Close();
    }
    m_messageHandler.Wait();
    m_openState.Close();
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::MessageLoop() {
    auto client = std::shared_ptr<Client>();
    while(m_openState.IsOpen()) {
      try {
        client = GetClient();
        while(true) {
          auto message = client->ReadMessage();
          if(auto slot = client->GetSlots().Find(*message)) {
            message->EmitSignal(slot, Ref(*client));
          }
        }
      } catch(const std::exception&) {
        auto lock = boost::lock_guard(m_mutex);
        if(client == m_client) {
          m_client = nullptr;
        }
      }
    }
  }
}

#endif
