#ifndef BEAM_SERVICE_PROTOCOL_CLIENT_HANDLER_HPP
#define BEAM_SERVICE_PROTOCOL_CLIENT_HANDLER_HPP
#include <utility>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Instantiates ServiceProtocolClients for a service.
   * @tparam B The type used to build ServiceProtocolClients.
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
      using ReconnectHandler =
        std::function<void (const std::shared_ptr<Client>& client)>;

      /**
       * Constructs a ServiceProtocolClientHandler.
       * @param builder Initializes the builder used for ServiceProtocolClients.
       */
      template<Initializes<B> BF>
      explicit ServiceProtocolClientHandler(BF&& builder);

      /**
       * Constructs a ServiceProtocolClientHandler.
       * @param builder Initializes the builder used for ServiceProtocolClients.
       * @param reconnect_handler The function to call to handle a reconnection
       *        event;
       */
      template<Initializes<B> BF>
      ServiceProtocolClientHandler(
        BF&& builder, ReconnectHandler reconnect_handler);

      ~ServiceProtocolClientHandler();

      /** Returns the slots used by the ServiceProtocolClients. */
      const ServiceSlots<Client>& get_slots() const;

      /** Returns the slots used by the ServiceProtocolClients. */
      ServiceSlots<Client>& get_slots();

      /** Returns the most recently instantiated client. */
      std::shared_ptr<Client> get_client();

      void close();

    private:
      RecursiveMutex m_mutex;
      local_ptr_t<B> m_builder;
      ServiceSlots<Client> m_slots;
      ReconnectHandler m_reconnect_handler;
      std::shared_ptr<Client> m_client;
      RoutineHandler m_message_handler;
      std::shared_ptr<typename ServiceProtocolClientBuilder::Timer>
        m_reconnect_timer;
      OpenState m_open_state;

      ServiceProtocolClientHandler(
        const ServiceProtocolClientHandler&) = delete;
      ServiceProtocolClientHandler& operator =(
        const ServiceProtocolClientHandler&) = delete;
      void message_loop();
  };

  template<typename B>
  template<Initializes<B> BF>
  ServiceProtocolClientHandler<B>::ServiceProtocolClientHandler(BF&& builder)
    : ServiceProtocolClientHandler(std::forward<BF>(builder),
        [] (const std::shared_ptr<Client>&) {}) {}

  template<typename B>
  template<Initializes<B> BF>
  ServiceProtocolClientHandler<B>::ServiceProtocolClientHandler(
      BF&& builder, ReconnectHandler reconnect_handler)
      : m_builder(std::forward<BF>(builder)),
        m_reconnect_handler(std::move(reconnect_handler)),
        m_client(m_builder->make_client(m_slots)) {
    m_message_handler =
      spawn(std::bind_front(&ServiceProtocolClientHandler::message_loop, this));
  }

  template<typename B>
  ServiceProtocolClientHandler<B>::~ServiceProtocolClientHandler() {
    close();
  }

  template<typename B>
  const ServiceSlots<typename ServiceProtocolClientHandler<B>::Client>&
      ServiceProtocolClientHandler<B>::get_slots() const {
    return m_slots;
  }

  template<typename B>
  ServiceSlots<typename ServiceProtocolClientHandler<B>::Client>&
      ServiceProtocolClientHandler<B>::get_slots() {
    return m_slots;
  }

  template<typename B>
  std::shared_ptr<typename ServiceProtocolClientHandler<B>::Client>
      ServiceProtocolClientHandler<B>::get_client() {
    while(true) {
      auto lock = boost::unique_lock(m_mutex);
      if(m_client) {
        return m_client;
      }
      m_open_state.ensure_open();
      try {
        auto client = [&] {
          auto releaser = release(lock);
          return m_builder->make_client(m_slots);
        }();
        if(m_client) {
          return m_client;
        }
        m_client = std::move(client);
        try {
          m_reconnect_handler(m_client);
        } catch(const std::exception&) {
          m_client = nullptr;
          m_open_state.close();
          throw;
        }
        return m_client;
      } catch(const ConnectException&) {
        auto reconnect_timer = std::shared_ptr(m_builder->make_timer());
        m_reconnect_timer = reconnect_timer;
        reconnect_timer->start();
        {
          auto releaser = release(lock);
          reconnect_timer->wait();
        }
        m_reconnect_timer = nullptr;
      } catch(const std::exception&) {
        m_open_state.close();
        throw;
      }
    }
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    auto [client, reconnect_timer] = [&] {
      auto lock = boost::lock_guard(m_mutex);
      return std::tuple(std::exchange(m_client, nullptr),
        std::exchange(m_reconnect_timer, nullptr));
    }();
    if(client) {
      client->close();
    }
    if(reconnect_timer) {
      reconnect_timer->cancel();
    }
    m_message_handler.wait();
    m_open_state.close();
  }

  template<typename B>
  void ServiceProtocolClientHandler<B>::message_loop() {
    auto client = std::shared_ptr<Client>();
    while(m_open_state.is_open()) {
      try {
        client = get_client();
        while(true) {
          auto message = client->read_message();
          if(auto slot = client->get_slots().find(*message)) {
            message->emit(slot, Ref(*client));
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
