#include "Beam/ServiceLocatorTests/ServiceLocatorClientTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace std;

namespace {
  LoginServiceResult AcceptLoginRequest(ServiceLocatorClientTester::
      ServiceProtocolServer::ServiceProtocolClient& client,
      const string& username, const string& password, bool& receivedRequest) {
    DirectoryEntry account(DirectoryEntry::Type::ACCOUNT, 0, "account");
    receivedRequest = true;
    return LoginServiceResult(account, "sessionid");
  }

  LoginServiceResult RejectLoginRequest(ServiceLocatorClientTester::
      ServiceProtocolServer::ServiceProtocolClient& client,
      const string& username, const string& password, bool& receivedRequest) {
    receivedRequest = true;
    throw ServiceRequestException();
  }
}

void ServiceLocatorClientTester::setUp() {
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
  m_serviceClient.Initialize(builder);
  m_protocolServer->Open();
  RegisterServiceLocatorServices(Store(m_protocolServer->GetSlots()));
  RegisterServiceLocatorMessages(Store(m_protocolServer->GetSlots()));
}

void ServiceLocatorClientTester::tearDown() {
  m_serviceClient.Reset();
  m_protocolServer.Reset();
  m_serverConnection.Reset();
}

void ServiceLocatorClientTester::TestLoginAccepted() {
  bool receivedRequest = false;
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
  bool receivedRequest = false;
  LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
    RejectLoginRequest, std::placeholders::_1, std::placeholders::_2,
    std::placeholders::_3, std::ref(receivedRequest)));
  m_serviceClient->SetCredentials("account", "password");
  CPPUNIT_ASSERT_THROW(m_serviceClient->Open(), ServiceRequestException);
}

void ServiceLocatorClientTester::TestMonitorDirectoryEntry() {}
