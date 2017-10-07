#include "Beam/UidServiceTests/UidServletTester.hpp"
#include <boost/functional/factory.hpp>

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace std;

void UidServletTester::setUp() {
  m_dataStore = std::make_shared<LocalUidDataStore>();
  auto serverConnection = std::make_unique<TestServerConnection>();
  m_clientProtocol.emplace(Initialize(string("test"), Ref(*serverConnection)),
    Initialize());
  RegisterUidServices(Store(m_clientProtocol->GetSlots()));
  m_container.emplace(m_dataStore, std::move(serverConnection),
    factory<std::unique_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void UidServletTester::tearDown() {
  m_clientProtocol.reset();
  m_container.reset();
  m_dataStore.reset();
}

void UidServletTester::TestReserveUidsService() {
  auto blockSize = std::uint64_t{100};
  auto firstUid = m_clientProtocol->SendRequest<ReserveUidsService>(blockSize);
  CPPUNIT_ASSERT(firstUid == 1);
  auto secondUid = m_clientProtocol->SendRequest<ReserveUidsService>(blockSize);
  CPPUNIT_ASSERT(secondUid == 101);
}
