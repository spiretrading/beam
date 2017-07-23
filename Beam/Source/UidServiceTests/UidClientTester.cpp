#include "Beam/UidServiceTests/UidClientTester.hpp"
#include <boost/functional/factory.hpp>
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace std;

void UidClientTester::setUp() {
  auto serverConnection = std::make_shared<TestServerConnection>();
  m_protocolServer.emplace(serverConnection,
    factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  m_protocolServer->Open();
  RegisterUidServices(Store(m_protocolServer->GetSlots()));
  TestServiceProtocolClientBuilder builder{
    [=] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>("test",
        Ref(*serverConnection));
    }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>()};
  m_uidClient.emplace(builder);
  m_uidClient->Open();
}

void UidClientTester::tearDown() {
  m_uidClient.reset();
  m_protocolServer.reset();
}

void UidClientTester::TestSingleUidRequest() {
  auto receivedRequest = false;
  auto INITIAL_UID = std::uint64_t{123};
  ReserveUidsService::AddSlot(Store(m_protocolServer->GetSlots()),
    [&] (auto& client, auto blockSize) {
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
    [&] (auto& client, auto blockSize) {
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
    [&] (auto& request, auto blockSize) {
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
    [&] (auto& request, auto blockSize) {
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
