#include <boost/functional/factory.hpp>
#include <boost/optional.hpp>
#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/ServicesTests/TestServices.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;

namespace {
  using TestServerConnection = LocalServerConnection<SharedBuffer>;
  using TestClientChannel = LocalClientChannel<SharedBuffer>;
  using TestServiceProtocolServer = ServiceProtocolServer<TestServerConnection*,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<TriggerTimer>>;
  using ClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<TestClientChannel, BinarySender<SharedBuffer>, NullEncoder>,
    TriggerTimer>;

  struct Fixture {
    TestServerConnection m_serverConnection;
    TestServiceProtocolServer m_protocolServer;
    ClientServiceProtocolClient m_clientProtocol;

    Fixture()
      : m_protocolServer(&m_serverConnection,
          factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot()),
        m_clientProtocol(Initialize("test", m_serverConnection), Initialize()) {
      RegisterTestServices(Store(m_protocolServer.GetSlots()));
      RegisterTestServices(Store(m_clientProtocol.GetSlots()));
    }
  };

  void OnVoidRequest(RequestToken<
      TestServiceProtocolServer::ServiceProtocolClient, VoidService>& request,
      int n, bool* callbackTriggered) {
    *callbackTriggered = true;
    request.SetResult();
  }
}

TEST_SUITE("ServiceProtocolServer") {
  TEST_CASE_FIXTURE(Fixture, "void_return_type") {
    auto callbackTriggered = false;
    VoidService::AddRequestSlot(Store(m_protocolServer.GetSlots()),
      std::bind(OnVoidRequest, std::placeholders::_1, std::placeholders::_2,
      &callbackTriggered));
  
    // Startup and invoke the service.
    m_clientProtocol.SendRequest<VoidService>(123);
    REQUIRE(callbackTriggered);
  }
}
