#include "Beam/UidServiceTests/UidServletTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace std;

void UidServletTester::setUp() {
  m_dataStore = std::make_shared<LocalUidDataStore>();
  m_serverConnection = std::make_shared<ServerConnection>();
  m_clientProtocol.Initialize(Initialize(string("test"),
    Ref(*m_serverConnection)), Initialize());
  RegisterUidServices(Store(m_clientProtocol->GetSlots()));
  m_container.Initialize(m_dataStore, m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void UidServletTester::tearDown() {
  m_clientProtocol.Reset();
  m_container.Reset();
  m_dataStore.reset();
}

void UidServletTester::TestReserveUidsService() {
  auto blockSize = std::uint64_t{100};
  auto firstUid = m_clientProtocol->SendRequest<ReserveUidsService>(blockSize);
  CPPUNIT_ASSERT(firstUid == 1);
  auto secondUid = m_clientProtocol->SendRequest<ReserveUidsService>(blockSize);
  CPPUNIT_ASSERT(secondUid == 101);
}
