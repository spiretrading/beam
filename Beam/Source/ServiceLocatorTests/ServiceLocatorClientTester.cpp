#include "Beam/ServiceLocatorTests/ServiceLocatorClientTester.hpp"
#include <boost/functional/factory.hpp>
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace std;

namespace {
  LoginServiceResult AcceptLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const string& username, const string& password, bool& receivedRequest) {
    DirectoryEntry account{DirectoryEntry::Type::ACCOUNT, 0, "account"};
    receivedRequest = true;
    return LoginServiceResult(account, "sessionid");
  }

  LoginServiceResult RejectLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const string& username, const string& password, bool& receivedRequest) {
    receivedRequest = true;
    throw ServiceRequestException();
  }
}

void ServiceLocatorClientTester::setUp() {
  auto serverConnection = std::make_shared<TestServerConnection>();
  m_protocolServer.emplace(serverConnection,
    factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  m_protocolServer->Open();
  RegisterServiceLocatorServices(Store(m_protocolServer->GetSlots()));
  RegisterServiceLocatorMessages(Store(m_protocolServer->GetSlots()));
  TestServiceProtocolClientBuilder builder{
    [=] {
      return std::make_unique<TestServiceProtocolClientBuilder::Channel>("test",
        Ref(*serverConnection));
    }, factory<unique_ptr<TestServiceProtocolClientBuilder::Timer>>()};
  m_serviceClient.emplace(builder);
}

void ServiceLocatorClientTester::tearDown() {
  m_serviceClient.reset();
  m_protocolServer.reset();
}

void ServiceLocatorClientTester::TestLoginAccepted() {
  auto receivedRequest = false;
  LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
    AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
    std::placeholders::_3, std::ref(receivedRequest)));
  m_serviceClient->SetCredentials("account", "password");
  CPPUNIT_ASSERT_NO_THROW(m_serviceClient->Open());
  CPPUNIT_ASSERT(receivedRequest);
  CPPUNIT_ASSERT(m_serviceClient->GetAccount().m_name == "account");
  CPPUNIT_ASSERT(m_serviceClient->GetSessionId() == "sessionid");
}

void ServiceLocatorClientTester::TestLoginRejected() {
  auto receivedRequest = false;
  LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
    RejectLoginRequest, std::placeholders::_1, std::placeholders::_2,
    std::placeholders::_3, std::ref(receivedRequest)));
  m_serviceClient->SetCredentials("account", "password");
  CPPUNIT_ASSERT_THROW(m_serviceClient->Open(), ServiceRequestException);
}

void ServiceLocatorClientTester::TestMonitorDirectoryEntry() {}
