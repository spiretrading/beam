module;
#include "Prelude.hpp"

module Beam;

namespace Beam::Tests {
  struct ServiceLocatorTestEnvironment::Impl {
    using ServiceProtocolServletContainer =
      Beam::ServiceProtocolServletContainer<
        MetaServiceLocatorServlet<LocalServiceLocatorDataStore*>,
        LocalServerConnection*, BinarySender<SharedBuffer>, NullEncoder,
        std::shared_ptr<TriggerTimer>>;
    using ServiceProtocolClientBuilder =
      Beam::ServiceProtocolClientBuilder<MessageProtocol<
        std::unique_ptr<LocalClientChannel>, BinarySender<SharedBuffer>,
        NullEncoder>, TriggerTimer>;
    LocalServiceLocatorDataStore m_data_store;
    LocalServerConnection m_server_connection;
    ServiceProtocolServletContainer m_container;
    boost::optional<ServiceLocatorClient> m_root;

    Impl()
      : m_container(&m_data_store, &m_server_connection,
          boost::factory<std::shared_ptr<TriggerTimer>>()) {}
  };

  ServiceLocatorTestEnvironment::ServiceLocatorTestEnvironment()
      : m_impl(std::make_unique<Impl>()) {
    m_impl->m_root.emplace(make_client("root", ""));
  }

  ServiceLocatorTestEnvironment::~ServiceLocatorTestEnvironment() {
    close();
  }

  void ServiceLocatorTestEnvironment::close() {
    m_impl->m_root->close();
    m_impl->m_container.close();
  }

  ServiceLocatorClient& ServiceLocatorTestEnvironment::get_root() {
    return *m_impl->m_root;
  }

  ServiceLocatorClient ServiceLocatorTestEnvironment::make_client(
      std::string username, std::string password) {
    return ServiceLocatorClient(std::in_place_type<
      ProtocolServiceLocatorClient<Impl::ServiceProtocolClientBuilder>>,
      std::move(username), std::move(password),
      Impl::ServiceProtocolClientBuilder(
        std::bind_front(boost::factory<
          std::unique_ptr<Impl::ServiceProtocolClientBuilder::Channel>>(),
          "test_service_locator_client", std::ref(m_impl->m_server_connection)),
        boost::factory<
          std::unique_ptr<Impl::ServiceProtocolClientBuilder::Timer>>()));
  }

  ServiceLocatorClient ServiceLocatorTestEnvironment::make_client(
      const std::string& session_id, unsigned int key) {
    return ServiceLocatorClient(std::in_place_type<
      ProtocolServiceLocatorClient<Impl::ServiceProtocolClientBuilder>>,
      session_id, key, Impl::ServiceProtocolClientBuilder(
        std::bind_front(boost::factory<
          std::unique_ptr<Impl::ServiceProtocolClientBuilder::Channel>>(),
          "test_service_locator_client", std::ref(m_impl->m_server_connection)),
        boost::factory<
          std::unique_ptr<Impl::ServiceProtocolClientBuilder::Timer>>()));
  }

  ServiceLocatorClient ServiceLocatorTestEnvironment::make_client() {
    return make_client("root", "");
  }
}
