#ifndef BEAM_SERVICES_TESTS_SERVICE_CLIENT_FIXTURE_HPP
#define BEAM_SERVICES_TESTS_SERVICE_CLIENT_FIXTURE_HPP
#include <memory>
#include <unordered_map>
#include <boost/functional/factory.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

namespace Beam::Tests {

  /**
   * Provides a fixture for testing ServiceProtocolClients.
   */
  struct ServiceClientFixture {
    std::shared_ptr<LocalServerConnection> m_server_connection;
    TestServiceProtocolServer m_server;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_handlers;

    /** Constructs a ServiceClientFixture. */
    ServiceClientFixture();

    /**
     * Makes a ServiceProtocolClient of the specified type.
     * @param args The arguments to forward to the constructor.
     * @return The ServiceProtocolClient.
     */
    template<typename T, typename... Args>
    std::unique_ptr<T> make_client(Args&&... args);

    /**
     * Registers a handler for the specified request type.
     * @param handler The handler to register.
     */
    template<typename T>
    void on_request(Beam::Details::request_slot_t<
      TestServiceProtocolServer::ServiceProtocolClient, T> handler);

    /**
     * Registers a handler for the specified message type.
     * @param handler The handler to register.
     */
    template<typename T>
    void on_message(Beam::Details::record_message_slot_t<
      RecordMessage<T, TestServiceProtocolServer::ServiceProtocolClient>>
        handler);
  };

  inline ServiceClientFixture::ServiceClientFixture()
    : m_server_connection(std::make_shared<LocalServerConnection>()),
      m_server(m_server_connection,
        boost::factory<std::unique_ptr<TriggerTimer>>(), NullSlot(),
        NullSlot()) {}

  template<typename T, typename... Args>
  std::unique_ptr<T> ServiceClientFixture::make_client(Args&&... args) {
    auto builder = TestServiceProtocolClientBuilder([=, this] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
        "test", *m_server_connection);
    }, boost::factory<
      std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
    return std::make_unique<T>(std::forward<Args>(args)..., builder);
  }

  template<typename T>
  void ServiceClientFixture::on_request(Beam::Details::request_slot_t<
      TestServiceProtocolServer::ServiceProtocolClient, T> handler) {
    using Slot = Beam::Details::request_slot_t<
      TestServiceProtocolServer::ServiceProtocolClient, T>;
    auto& stored_handler = m_handlers[typeid(T)];
    if(stored_handler) {
      *std::static_pointer_cast<Slot>(stored_handler) = std::move(handler);
    } else {
      auto shared_handler = std::make_shared<Slot>(std::move(handler));
      stored_handler = shared_handler;
      T::add_request_slot(out(m_server.get_slots()),
        [handler = std::move(shared_handler)] (auto&&... args) {
          try {
            (*handler)(std::forward<decltype(args)>(args)...);
          } catch(...) {
            boost::throw_with_location(ServiceRequestException("Test failed."));
          }
        });
    }
  }

  template<typename T>
  void ServiceClientFixture::on_message(Beam::Details::record_message_slot_t<
      RecordMessage<T, TestServiceProtocolServer::ServiceProtocolClient>>
        handler) {
    using Slot = Beam::Details::record_message_slot_t<
      RecordMessage<T, TestServiceProtocolServer::ServiceProtocolClient>>;
    auto& stored_handler = m_handlers[typeid(T)];
    if(stored_handler) {
      *std::static_pointer_cast<Slot>(stored_handler) = std::move(handler);
    } else {
      auto shared_handler = std::make_shared<Slot>(std::move(handler));
      stored_handler = shared_handler;
      add_message_slot<T>(out(m_server.get_slots()),
        [handler = std::move(shared_handler)] (auto&&... args) {
          try {
            (*handler)(std::forward<decltype(args)>(args)...);
          } catch(...) {
            boost::throw_with_location(ServiceRequestException("Test failed."));
          }
        });
    }
  }
}

#endif
