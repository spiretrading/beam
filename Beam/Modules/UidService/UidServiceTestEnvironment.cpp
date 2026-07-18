module;
#include "Prelude.hpp"

module Beam;

namespace Beam::Tests {
  struct UidServiceTestEnvironment::Impl {
    using ServiceProtocolServletContainer =
      Beam::ServiceProtocolServletContainer<
        MetaUidServlet<LocalUidDataStore*>, LocalServerConnection*,
        BinarySender<SharedBuffer>, NullEncoder,
        std::shared_ptr<TriggerTimer>>;
    using ServiceProtocolClientBuilder =
      Beam::ServiceProtocolClientBuilder<
        MessageProtocol<std::unique_ptr<LocalClientChannel>,
          BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;
    LocalUidDataStore m_data_store;
    LocalServerConnection m_server_connection;
    ServiceProtocolServletContainer m_container;

    Impl()
      : m_container(&m_data_store, &m_server_connection,
          boost::factory<std::shared_ptr<TriggerTimer>>()) {}
  };

  UidServiceTestEnvironment::UidServiceTestEnvironment()
    : m_impl(std::make_unique<Impl>()) {}

  UidServiceTestEnvironment::~UidServiceTestEnvironment() {
    close();
  }

  UidClient UidServiceTestEnvironment::make_client() {
    return UidClient(std::in_place_type<
      ServiceUidClient<Impl::ServiceProtocolClientBuilder>>,
      Impl::ServiceProtocolClientBuilder(std::bind_front(boost::factory<
        std::unique_ptr<Impl::ServiceProtocolClientBuilder::Channel>>(),
        "test_uid_client", std::ref(m_impl->m_server_connection)),
        std::bind_front(boost::factory<
          std::unique_ptr<Impl::ServiceProtocolClientBuilder::Timer>>())));
  }

  void UidServiceTestEnvironment::close() {
    m_impl->m_container.close();
  }
}
