#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"
#include "Beam/UidService/UidClient.hpp"

using namespace Beam;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace boost;

namespace {
  struct Fixture {
    using TestUidClient = UidClient<TestServiceProtocolClientBuilder>;

    boost::optional<TestServiceProtocolServer> m_protocolServer;
    boost::optional<TestUidClient> m_uidClient;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_protocolServer.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      m_protocolServer->Open();
      RegisterUidServices(Store(m_protocolServer->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", Ref(*serverConnection));
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_uidClient.emplace(builder);
      m_uidClient->Open();
    }
  };
}

TEST_SUITE("UidClient") {
  TEST_CASE_FIXTURE(Fixture, "single_uid_request") {
    auto receivedRequest = false;
    auto INITIAL_UID = std::uint64_t(123);
    ReserveUidsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, auto blockSize) {
        REQUIRE(blockSize > 0);
        receivedRequest = true;
        return INITIAL_UID;
      });
    auto uidResult = m_uidClient->LoadNextUid();
    REQUIRE(receivedRequest);
    REQUIRE(uidResult == INITIAL_UID);
  }

  TEST_CASE_FIXTURE(Fixture, "sequential_uid_requests") {
    auto requestCount = 0;
    auto INITIAL_UID = std::uint64_t(123);
    ReserveUidsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, auto blockSize) {
        REQUIRE(blockSize > 0);
        ++requestCount;
        return INITIAL_UID;
      });
    auto uidResultA = m_uidClient->LoadNextUid();
    REQUIRE(requestCount == 1);
    REQUIRE(uidResultA == INITIAL_UID);
    auto uidResultB = m_uidClient->LoadNextUid();
    REQUIRE(requestCount == 1);
    REQUIRE(uidResultB == INITIAL_UID + 1);
  }

  TEST_CASE_FIXTURE(Fixture, "simultaneous_uid_requests") {
    auto requestCount = 0;
    auto INITIAL_UID = std::uint64_t(123);
    ReserveUidsService::AddRequestSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& request, auto blockSize) {
        REQUIRE(blockSize > 0);
        ++requestCount;
        request.SetResult(INITIAL_UID);
      });
    auto uidResultA = m_uidClient->LoadNextUid();
    REQUIRE(uidResultA == INITIAL_UID);
    auto uidResultB = m_uidClient->LoadNextUid();
    REQUIRE(uidResultB == INITIAL_UID + 1);
    REQUIRE(requestCount == 1);
  }

  TEST_CASE_FIXTURE(Fixture, "multiple_server_requests") {
    auto requestCount = 0;
    auto INITIAL_UID = std::uint64_t(123);
    auto requestBlockSize = std::uint64_t();
    ReserveUidsService::AddRequestSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& request, auto blockSize) {
        REQUIRE(blockSize > 0);
        requestBlockSize = blockSize;
        ++requestCount;
        if(requestCount == 1) {
          request.SetResult(INITIAL_UID);
        } else {
          request.SetResult(1000 * INITIAL_UID);
        }
      });
    auto uidResultA = m_uidClient->LoadNextUid();
    auto initialBlockResults = std::vector<std::uint64_t>();

    // Exhaust all of the UIDs from the initial request.
    for(auto i = std::uint64_t(1); i < requestBlockSize; ++i) {
      auto uidResult = m_uidClient->LoadNextUid();
      initialBlockResults.push_back(uidResult);
    }
    auto secondaryBlockResults = std::vector<std::uint64_t>();

    // Request UIDs from a new block.
    for(auto i = std::uint64_t(0); i < requestBlockSize; ++i) {
      auto uidResult = m_uidClient->LoadNextUid();
      secondaryBlockResults.push_back(uidResult);
    }
    REQUIRE(requestCount == 2);
    REQUIRE(uidResultA == INITIAL_UID);
    auto counter = std::uint64_t(1);
    for(auto uid : initialBlockResults) {
      REQUIRE(uid == INITIAL_UID + counter);
      ++counter;
    }
    counter = 0;
    for(auto uid : secondaryBlockResults) {
      REQUIRE(uid == 1000 * INITIAL_UID + counter);
      ++counter;
    }
  }
}
