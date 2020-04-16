#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace boost;

namespace {
  struct Fixture {
    using UidServletContainer = TestServiceProtocolServletContainer<
      MetaUidServlet<LocalUidDataStore*>>;

    LocalUidDataStore m_dataStore;
    boost::optional<UidServletContainer> m_container;
    boost::optional<TestServiceProtocolClient> m_clientProtocol;

    Fixture() {
      auto serverConnection = std::make_unique<TestServerConnection>();
      m_clientProtocol.emplace(Initialize(std::string("test"),
        Ref(*serverConnection)), Initialize());
      RegisterUidServices(Store(m_clientProtocol->GetSlots()));
      m_container.emplace(&m_dataStore, std::move(serverConnection),
        factory<std::unique_ptr<TriggerTimer>>());
      m_container->Open();
      m_clientProtocol->Open();
    }
  };
}

TEST_SUITE("UidServlet") {
  TEST_CASE_FIXTURE(Fixture, "reserve_uids_service") {
    auto blockSize = std::uint64_t{100};
    auto firstUid = m_clientProtocol->SendRequest<ReserveUidsService>(
      blockSize);
    REQUIRE(firstUid == 1);
    auto secondUid = m_clientProtocol->SendRequest<ReserveUidsService>(
      blockSize);
    REQUIRE(secondUid == 101);
  }
}
