#include "Beam/ServicesTests/ServiceProtocolServerTester.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;
using namespace std;

namespace {
  void OnVoidRequest(RequestToken<
      ServiceProtocolServerTester::ServiceProtocolServer::ServiceProtocolClient,
      VoidService>& request, int n, bool* callbackTriggered) {
    *callbackTriggered = true;
    request.SetResult();
  }
}

void ServiceProtocolServerTester::setUp() {
  auto serverConnection = std::make_unique<ServerConnection>();
  m_clientProtocol.emplace(Initialize(string("test"),
    Ref(*serverConnection)), Initialize());
  RegisterTestServices(Store(m_clientProtocol->GetSlots()));
  m_protocolServer.emplace(std::move(serverConnection),
    factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
  RegisterTestServices(Store(m_protocolServer->GetSlots()));
  m_protocolServer->Open();
  m_clientProtocol->Open();
}

void ServiceProtocolServerTester::tearDown() {
  m_clientProtocol = std::nullopt;
  m_protocolServer = std::nullopt;
}

void ServiceProtocolServerTester::TestVoidReturnType() {
  RoutineHandler task = Spawn(
    [&] {
      bool callbackTriggered = false;
      VoidService::AddRequestSlot(Store(m_protocolServer->GetSlots()),
        std::bind(OnVoidRequest, std::placeholders::_1, std::placeholders::_2,
        &callbackTriggered));

      // Startup and invoke the service.
      m_clientProtocol->SendRequest<VoidService>(123);
      CPPUNIT_ASSERT(callbackTriggered);
      m_protocolServer->Close();
    });
  task.Wait();
}
