#ifndef BEAM_SERVICE_PROTOCOL_CLIENT_HANDLER_HPP
#define BEAM_SERVICE_PROTOCOL_CLIENT_HANDLER_HPP
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Utilities/ReportException.hpp"

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

      ~ServiceProtocolClientHandler();

      /** Returns the slots used by the ServiceProtocolClients. */
      const ServiceSlots<Client>& GetSlots() const;

      /** Returns the slots used by the ServiceProtocolClients. */
      ServiceSlots<Client>& GetSlots();

      /** Returns the most recently instantiated client. */
      std::shared_ptr<Client> GetClient();

      /**
       * Sets the function to call to handle a reconnection.
       * @param reconnectHandler The reconnection handler.
       */
      void SetReconnectHandler(const ReconnectHandler& reconnectHandler);

      void Close();

    private:
      Threading::Mutex m_mutex;
      GetOptionalLocalPtr<B> m_builder;
      ServiceSlots<Client> m_slots;
      ReconnectHandler m_reconnectHandler;
      std::shared_ptr<Client> m_client;
      Routines::RoutineHandlerGroup m_messageHandlers;
      IO::OpenState m_openState;

      ServiceProtocolClientHandler(
        const ServiceProtocolClientHandler&) = delete;
      ServiceProtocolClientHandler& operator =(
        const ServiceProtocolClientHandler&) = delete;
      void BuildClient();
      void MessageLoop(std::shared_ptr<Client> client);
  };

  template<typename B>
  template<typename BF>
  ServiceProtocolClientHandler<B>::ServiceProtocolClientHandler(BF&& builder)
      : m_builder(std::forward<BF>(builder)),
        m_reconnectHandler([] (const std::shared_ptr<Client>&) {}) {
    try {
      BuildClient();
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

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
    auto client = [&] {
      auto lock = boost::lock_guard(m_mutex);
      if(m_client == nullptr) {
        BuildClient();
      }
      return m_client;
    }();
    return client;
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::SetReconnectHandler(
      const ReconnectHandler& reconnectHandler) {
    m_reconnectHandler = reconnectHandler;
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto client = [&] {
      auto lock = boost::lock_guard(m_mutex);
      return m_client;
    }();
    if(client) {
      client->Close();
    }
    m_messageHandlers.Wait();
    m_openState.Close();
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::BuildClient() {
    m_client = [&] {
      while(true) {
        m_openState.EnsureOpen();
        try {
          auto client = std::shared_ptr(m_builder->Build(m_slots));
          m_builder->Open(*client);
          return client;
        } catch(const IO::ConnectException&) {
          if(!m_openState.IsOpen()) {
            throw IO::NotConnectedException();
          }
        } catch(const std::exception&) {
          m_openState.Close();
          BOOST_RETHROW;
        }
        Routines::Defer();
      }
    }();
    if(m_openState.IsOpen()) {
      try {
        m_reconnectHandler(m_client);
      } catch(const std::exception&) {
        m_openState.Close();
        return;
      }
    }
    m_messageHandlers.Spawn(
      std::bind(&ServiceProtocolClientHandler::MessageLoop, this, m_client));
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::MessageLoop(
      std::shared_ptr<Client> client) {
    try {
      while(true) {
        auto message = client->ReadMessage();
        if(auto slot = client->GetSlots().Find(*message)) {
          message->EmitSignal(slot, Ref(*client));
        }
      }
    } catch(const std::exception&) {
      auto lock = boost::lock_guard(m_mutex);
      try {
        BuildClient();
      } catch(const IO::NotConnectedException&) {
        return;
      }
    }
  }
}

#endif
