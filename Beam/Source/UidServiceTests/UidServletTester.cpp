#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

namespace {
  struct Fixture {
    using ServletContainer =
      TestServiceProtocolServletContainer<MetaUidServlet<LocalUidDataStore*>>;
    LocalUidDataStore m_data_store;
    optional<ServletContainer> m_container;
    optional<TestServiceProtocolClient> m_client_protocol;

    Fixture() {
      auto server_connection = std::make_shared<LocalServerConnection>();
      m_container.emplace(&m_data_store, server_connection,
        factory<std::unique_ptr<TriggerTimer>>());
      m_client_protocol.emplace(std::make_unique<LocalClientChannel>(
        "test", *server_connection), init());
      register_uid_services(out(m_client_protocol->get_slots()));
    }
  };
}

TEST_SUITE("UidServlet") {
  TEST_CASE("reserve_uids_service") {
    auto fixture = Fixture();
    auto block_size = std::uint64_t(100);
    auto first_uid =
      fixture.m_client_protocol->send_request<ReserveUidsService>(block_size);
    REQUIRE(first_uid == 1);
    auto second_uid =
      fixture.m_client_protocol->send_request<ReserveUidsService>(block_size);
    REQUIRE(second_uid == 101);
  }
}
