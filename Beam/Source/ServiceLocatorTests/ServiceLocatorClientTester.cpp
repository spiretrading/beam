#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;

namespace {
  LoginServiceResult AcceptLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const std::string& username, const std::string& password,
      bool& receivedRequest) {
    auto account = DirectoryEntry::MakeAccount(0, "account");
    receivedRequest = true;
    return LoginServiceResult(account, "sessionid");
  }

  LoginServiceResult RejectLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const std::string& username, const std::string& password,
      bool& receivedRequest) {
    receivedRequest = true;
    throw ServiceRequestException();
  }

  struct Fixture {
    using TestServiceLocatorClient = ServiceLocatorClient<
      TestServiceProtocolClientBuilder>;
    boost::optional<TestServiceProtocolServer> m_protocolServer;
    boost::optional<TestServiceLocatorClient> m_serviceClient;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_protocolServer.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      m_protocolServer->Open();
      RegisterServiceLocatorServices(Store(m_protocolServer->GetSlots()));
      RegisterServiceLocatorMessages(Store(m_protocolServer->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          return std::make_unique<TestServiceProtocolClientBuilder::Channel>(
            "test", Ref(*serverConnection));
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_serviceClient.emplace(builder);
    }
  };
}

TEST_SUITE("ServiceLocatorClient") {
  TEST_CASE_FIXTURE(Fixture, "login_accepted") {
    auto receivedRequest = false;
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::ref(receivedRequest)));
    m_serviceClient->SetCredentials("account", "password");
    REQUIRE_NOTHROW(m_serviceClient->Open());
    REQUIRE(receivedRequest);
    REQUIRE(m_serviceClient->GetAccount().m_name == "account");
    REQUIRE(m_serviceClient->GetSessionId() == "sessionid");
  }

  TEST_CASE_FIXTURE(Fixture, "login_rejected") {
    auto receivedRequest = false;
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      RejectLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::ref(receivedRequest)));
    m_serviceClient->SetCredentials("account", "password");
    REQUIRE_THROWS_AS(m_serviceClient->Open(), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_directory_entry") {}
}
