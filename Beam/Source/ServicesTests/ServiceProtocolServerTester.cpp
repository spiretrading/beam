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
  using TestServiceProtocolServer = ServiceProtocolServer<
    std::unique_ptr<TestServerConnection>, BinarySender<SharedBuffer>,
    NullEncoder, std::shared_ptr<TriggerTimer>>;
  using ClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<TestClientChannel, BinarySender<SharedBuffer>, NullEncoder>,
    TriggerTimer>;

  struct Fixture {
    boost::optional<TestServiceProtocolServer> m_protocolServer;
    boost::optional<ClientServiceProtocolClient> m_clientProtocol;

    Fixture() {
      auto serverConnection = std::make_unique<TestServerConnection>();
      m_clientProtocol.emplace(Initialize(std::string("test"),
        Ref(*serverConnection)), Initialize());
      RegisterTestServices(Store(m_clientProtocol->GetSlots()));
      m_protocolServer.emplace(std::move(serverConnection),
        factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      RegisterTestServices(Store(m_protocolServer->GetSlots()));
      m_protocolServer->Open();
      m_clientProtocol->Open();
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
    auto task = RoutineHandler(Spawn(
      [&] {
        auto callbackTriggered = false;
        VoidService::AddRequestSlot(Store(m_protocolServer->GetSlots()),
          std::bind(OnVoidRequest, std::placeholders::_1, std::placeholders::_2,
          &callbackTriggered));
  
        // Startup and invoke the service.
        m_clientProtocol->SendRequest<VoidService>(123);
        REQUIRE(callbackTriggered);
        m_protocolServer->Close();
      }));
    task.Wait();
  }
}
