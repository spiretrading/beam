#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;

namespace {
  LoginServiceResult AcceptLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const std::string& username, const std::string& password) {
    if(username == "account" && password == "password") {
      return LoginServiceResult(DirectoryEntry::MakeAccount(0, "account"),
        "sessionid");
    }
    throw ServiceRequestException();
  }

  struct Fixture {
    using TestServiceLocatorClient = ServiceLocatorClient<
      TestServiceProtocolClientBuilder>;
    std::shared_ptr<TestServerConnection> m_serverConnection;
    optional<TestServiceProtocolServer> m_protocolServer;
    optional<TestServiceLocatorClient> m_serviceClient;
    std::vector<TestServiceProtocolClientBuilder::Channel*> m_clientChannels;

    Fixture()
        : m_serverConnection(std::make_shared<TestServerConnection>()) {
      m_protocolServer.emplace(m_serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      RegisterServiceLocatorServices(Store(m_protocolServer->GetSlots()));
      RegisterServiceLocatorMessages(Store(m_protocolServer->GetSlots()));
      LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
        AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3));
      auto builder = TestServiceProtocolClientBuilder(
        [this] {
          auto channel = std::make_unique<
            TestServiceProtocolClientBuilder::Channel>("test",
            *m_serverConnection);
          m_clientChannels.push_back(channel.get());
          return channel;
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_serviceClient.emplace("account", "password", builder);
    }
  };
}

TEST_SUITE("ServiceLocatorClient") {
  TEST_CASE_FIXTURE(Fixture, "login_accepted") {
    REQUIRE(m_serviceClient->GetAccount().m_name == "account");
    REQUIRE(m_serviceClient->GetSessionId() == "sessionid");
  }

  TEST_CASE_FIXTURE(Fixture, "login_rejected") {
    auto builder = TestServiceProtocolClientBuilder(
      [this] {
        return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
          "test", *m_serverConnection);
      }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
    REQUIRE_THROWS_AS(TestServiceLocatorClient("account", "bad_password",
      builder), ConnectException);
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_accounts") {
    auto receivedUnmonitor = Async<void>();
    UnmonitorAccountsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client) {
        receivedUnmonitor.GetEval().SetResult();
      });
    auto testAccounts = std::vector<DirectoryEntry>();
    testAccounts.push_back(DirectoryEntry::MakeAccount(123, "accountA"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(124, "accountB"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(125, "accountC"));
    auto serverSideClient =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    MonitorAccountsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client) {
        serverSideClient = &client;
        return testAccounts;
      });
    auto accountQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClient->MonitorAccounts(accountQueue);
    auto update = accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[0], AccountUpdate::Type::ADDED});
    update = accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[1], AccountUpdate::Type::ADDED});
    update = accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[2], AccountUpdate::Type::ADDED});
    SendRecordMessage<AccountUpdateMessage>(*serverSideClient,
      AccountUpdate{testAccounts[0], AccountUpdate::Type::DELETED});
    update = accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[0], AccountUpdate::Type::DELETED});
    auto duplicateQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClient->MonitorAccounts(duplicateQueue);
    update = duplicateQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[1], AccountUpdate::Type::ADDED});
    update = duplicateQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[2], AccountUpdate::Type::ADDED});
    accountQueue->Break();
    duplicateQueue->Break();
    SendRecordMessage<AccountUpdateMessage>(*serverSideClient,
      AccountUpdate{testAccounts[1], AccountUpdate::Type::DELETED});
    REQUIRE_NOTHROW(receivedUnmonitor.Get());
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_accounts_reconnect") {
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
    auto testAccounts = std::vector<DirectoryEntry>();
    testAccounts.push_back(DirectoryEntry::MakeAccount(123, "accountA"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(124, "accountB"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(125, "accountC"));
    MonitorAccountsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client) {
        return testAccounts;
      });
    auto accountQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClient->MonitorAccounts(accountQueue);
    for(auto i = std::size_t(0); i != testAccounts.size(); ++i) {
      accountQueue->Pop();
    }
    testAccounts.push_back(DirectoryEntry::MakeAccount(135, "accountD"));
    m_clientChannels.back()->GetConnection().Close();
    auto recoveredAccount = accountQueue->Pop();
    REQUIRE(recoveredAccount ==
      AccountUpdate{testAccounts.back(), AccountUpdate::Type::ADDED});
    m_serviceClient->Close();
    REQUIRE_THROWS_AS(accountQueue->Pop(), PipeBrokenException);
  }

  TEST_CASE_FIXTURE(Fixture, "register_service_reconnect") {
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3));
    auto nextId = 1;
    auto registeredServices = std::vector<ServiceEntry>();
    auto recoveryToken = Async<void>();
    RegisterService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, const std::string& name,
          const JsonObject& properties) {
        ++nextId;
        auto service = ServiceEntry(name, properties, nextId,
          DirectoryEntry::MakeAccount(12, "service"));
        registeredServices.push_back(service);
        if(nextId == 5) {
          recoveryToken.GetEval().SetResult();
        }
        return service;
      });
    auto p1 = JsonObject();
    p1.Set("meta1", 12);
    p1.Set("meta2", "alpha");
    auto s1 = m_serviceClient->Register("s1", p1);
    auto p2 = JsonObject();
    p2.Set("meta3", "beta");
    p2.Set("meta4", false);
    auto s2 = m_serviceClient->Register("s2", p2);
    registeredServices.clear();
    m_clientChannels.back()->GetConnection().Close();
    recoveryToken.Get();
    REQUIRE(registeredServices.size() == 2);
    REQUIRE(registeredServices[0].GetAccount() == s1.GetAccount());
    REQUIRE(registeredServices[0].GetName() == s1.GetName());
    REQUIRE(registeredServices[0].GetProperties() == s1.GetProperties());
    REQUIRE(registeredServices[1].GetAccount() == s2.GetAccount());
    REQUIRE(registeredServices[1].GetName() == s2.GetName());
    REQUIRE(registeredServices[1].GetProperties() == s2.GetProperties());
  }
}
