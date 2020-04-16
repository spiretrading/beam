#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/ServicesTests/TestServlet.hpp"
#include "Beam/Threading/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::Threading;
using namespace boost;

namespace {
  using TestServerConnection = LocalServerConnection<SharedBuffer>;
  using TestClientChannel = LocalClientChannel<SharedBuffer>;
  using TestServiceProtocolServletContainer = ServiceProtocolServletContainer<
    MetaTestServlet, std::unique_ptr<TestServerConnection>,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<TriggerTimer>>;
  using ClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<TestClientChannel, BinarySender<SharedBuffer>, NullEncoder>,
    TriggerTimer>;

  struct Fixture {
    optional<TestServiceProtocolServletContainer> m_container;
    optional<ClientServiceProtocolClient> m_clientProtocol;

    Fixture() {
      auto serverConnection = std::make_unique<TestServerConnection>();
      m_clientProtocol.emplace(Initialize(std::string("test"),
        Ref(*serverConnection)), Initialize());
      RegisterTestServices(Store(m_clientProtocol->GetSlots()));
      m_container.emplace(Initialize(), std::move(serverConnection),
        factory<std::shared_ptr<TriggerTimer>>());
      m_container->Open();
      m_clientProtocol->Open();
    }
  };
}

TEST_SUITE("ServiceProtocolServletContainer") {
  TEST_CASE_FIXTURE(Fixture, "void_return_type") {
    auto task = RoutineHandler(Spawn(
      [&] {
        m_clientProtocol->SendRequest<VoidService>(123);
        m_container->Close();
      }));
    task.Wait();
  }
}
