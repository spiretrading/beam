#include "Beam/ServicesTests/ServiceProtocolServletContainerTester.hpp"
#include <boost/functional/factory.hpp>
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace std;

void ServiceProtocolServletContainerTester::setUp() {
  auto serverConnection = std::make_unique<ServerConnection>();
  m_clientProtocol.emplace(Initialize(string("test"),
    Ref(*serverConnection)), Initialize());
  RegisterTestServices(Store(m_clientProtocol->GetSlots()));
  m_container.emplace(Initialize(), std::move(serverConnection),
    factory<std::shared_ptr<TriggerTimer>>());
  m_container->Open();
  m_clientProtocol->Open();
}

void ServiceProtocolServletContainerTester::tearDown() {
  m_clientProtocol = std::nullopt;
  m_container = std::nullopt;
}

void ServiceProtocolServletContainerTester::TestVoidReturnType() {
  RoutineHandler task = Spawn(
    [&] {
      m_clientProtocol->SendRequest<VoidService>(123);
      m_container->Close();
    });
  task.Wait();
}
