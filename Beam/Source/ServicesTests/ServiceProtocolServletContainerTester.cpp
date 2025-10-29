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
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

namespace {
  using TestServiceProtocolServletContainer = ServiceProtocolServletContainer<
    MetaTestServlet, LocalServerConnection*, BinarySender<SharedBuffer>,
    NullEncoder, std::shared_ptr<TriggerTimer>>;
  using ClientServiceProtocolClient = ServiceProtocolClient<MessageProtocol<
    LocalClientChannel, BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;

  struct Fixture {
    LocalServerConnection m_server_connection;
    TestServiceProtocolServletContainer m_container;
    ClientServiceProtocolClient m_client_protocol;

    Fixture()
        : m_container(init(), &m_server_connection,
            factory<std::shared_ptr<TriggerTimer>>()),
          m_client_protocol(init("test", m_server_connection), init()) {
      register_test_services(out(m_client_protocol.get_slots()));
    }
  };
}

TEST_SUITE("ServiceProtocolServletContainer") {
  TEST_CASE_FIXTURE(Fixture, "void_return_type") {
    m_client_protocol.send_request<VoidService>(123);
  }
}
