#include "Beam/UidServiceTests/UidClientTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Beam/ServiceLocator/NullAuthenticator.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace std;

void UidClientTester::setUp() {
  m_serverConnection.Initialize();
  m_protocolServer.Initialize(&*m_serverConnection,
    factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  ServiceProtocolClientBuilder builder(
    [&] {
      return std::make_unique<ServiceProtocolClientBuilder::Channel>(("test"),
        Ref(*m_serverConnection));
    },
    [&] {
      return std::make_unique<ServiceProtocolClientBuilder::Timer>();
    });
  m_uidClient.Initialize(builder);
  m_protocolServer->Open();
  m_uidClient->Open();
  RegisterUidServices(Store(m_protocolServer->GetSlots()));
}

void UidClientTester::tearDown() {
  m_uidClient.Reset();
  m_protocolServer.Reset();
  m_serverConnection.Reset();
}

void UidClientTester::TestSingleUidRequest() {
  auto receivedRequest = false;
  auto INITIAL_UID = std::uint64_t{123};
  ReserveUidsService::AddSlot(Store(m_protocolServer->GetSlots()),
    [&] (ServiceProtocolServer::ServiceProtocolClient& client,
        std::uint64_t blockSize) -> std::uint64_t {
      CPPUNIT_ASSERT(blockSize > 0);
      receivedRequest = true;
      return INITIAL_UID;
    });
  auto uidResult = m_uidClient->LoadNextUid();
  CPPUNIT_ASSERT(receivedRequest);
  CPPUNIT_ASSERT(uidResult == INITIAL_UID);
}

void UidClientTester::TestSequentialUidRequests() {
  auto requestCount = 0;
  auto INITIAL_UID = std::uint64_t{123};
  ReserveUidsService::AddSlot(Store(m_protocolServer->GetSlots()),
    [&] (ServiceProtocolServer::ServiceProtocolClient& client,
        std::uint64_t blockSize) -> std::uint64_t {
      CPPUNIT_ASSERT(blockSize > 0);
      ++requestCount;
      return INITIAL_UID;
    });
  auto uidResultA = m_uidClient->LoadNextUid();
  CPPUNIT_ASSERT(requestCount == 1);
  CPPUNIT_ASSERT(uidResultA == INITIAL_UID);
  auto uidResultB = m_uidClient->LoadNextUid();
  CPPUNIT_ASSERT(requestCount == 1);
  CPPUNIT_ASSERT(uidResultB == INITIAL_UID + 1);
}

void UidClientTester::TestSimultaneousUidRequests() {
  auto requestCount = 0;
  auto INITIAL_UID = std::uint64_t{123};
  ReserveUidsService::AddRequestSlot(Store(m_protocolServer->GetSlots()),
    [&] (RequestToken<ServiceProtocolServer::ServiceProtocolClient,
        ReserveUidsService>& request, std::uint64_t blockSize) {
      CPPUNIT_ASSERT(blockSize > 0);
      ++requestCount;
      request.SetResult(INITIAL_UID);
    });
  auto uidResultA = m_uidClient->LoadNextUid();
  CPPUNIT_ASSERT(uidResultA == INITIAL_UID);
  auto uidResultB = m_uidClient->LoadNextUid();
  CPPUNIT_ASSERT(uidResultB == INITIAL_UID + 1);
  CPPUNIT_ASSERT(requestCount == 1);
}

void UidClientTester::TestMultipleServerRequests() {
  auto requestCount = 0;
  auto INITIAL_UID = std::uint64_t{123};
  std::uint64_t requestBlockSize;
  ReserveUidsService::AddRequestSlot(Store(m_protocolServer->GetSlots()),
    [&] (RequestToken<ServiceProtocolServer::ServiceProtocolClient,
        ReserveUidsService>& request, std::uint64_t blockSize) {
      CPPUNIT_ASSERT(blockSize > 0);
      requestBlockSize = blockSize;
      ++requestCount;
      if(requestCount == 1) {
        request.SetResult(INITIAL_UID);
      } else {
        request.SetResult(1000 * INITIAL_UID);
      }
    });
  auto uidResultA = m_uidClient->LoadNextUid();
  vector<std::uint64_t> initialBlockResults;

  // Exhaust all of the UIDs from the initial request.
  for(auto i = std::uint64_t{1}; i < requestBlockSize; ++i) {
    auto uidResult = m_uidClient->LoadNextUid();
    initialBlockResults.push_back(uidResult);
  }
  vector<std::uint64_t> secondaryBlockResults;

  // Request UIDs from a new block.
  for(auto i = std::uint64_t{0}; i < requestBlockSize; ++i) {
    auto uidResult = m_uidClient->LoadNextUid();
    secondaryBlockResults.push_back(uidResult);
  }
  CPPUNIT_ASSERT(requestCount == 2);
  CPPUNIT_ASSERT(uidResultA == INITIAL_UID);
  auto counter = std::uint64_t{1};
  for(auto uid : initialBlockResults) {
    CPPUNIT_ASSERT(uid == INITIAL_UID + counter);
    ++counter;
  }
  counter = 0;
  for(auto uid : secondaryBlockResults) {
    CPPUNIT_ASSERT(uid == 1000 * INITIAL_UID + counter);
    ++counter;
  }
}
