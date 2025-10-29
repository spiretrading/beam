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
#include "Beam/SignalHandling/NullSlot.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;

namespace {
  using TestServiceProtocolServer =
    ServiceProtocolServer<LocalServerConnection*,
      BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<TriggerTimer>>;
  using ClientServiceProtocolClient = ServiceProtocolClient<MessageProtocol<
    LocalClientChannel, BinarySender<SharedBuffer>, NullEncoder>, TriggerTimer>;

  struct Fixture {
    LocalServerConnection m_server_connection;
    TestServiceProtocolServer m_protocol_server;
    ClientServiceProtocolClient m_client_protocol;

    Fixture()
      : m_protocol_server(&m_server_connection,
          factory<std::shared_ptr<TriggerTimer>>(), NullSlot(), NullSlot()),
        m_client_protocol(init("test", m_server_connection), init()) {
      register_test_services(out(m_protocol_server.get_slots()));
      register_test_services(out(m_client_protocol.get_slots()));
    }
  };

  void on_void_request(RequestToken<
      TestServiceProtocolServer::ServiceProtocolClient, VoidService>& request,
      int n, bool* callback_triggered) {
    *callback_triggered = true;
    request.set();
  }

  void on_identity_request(RequestToken<
      TestServiceProtocolServer::ServiceProtocolClient, IdentityService>&
        request, int n) {
    if(n == 0) {
      throw ServiceRequestException("Exception.");
    }
    request.set(n);
  }
}

TEST_SUITE("ServiceProtocolServer") {
  TEST_CASE_FIXTURE(Fixture, "void_return_type") {
    auto callback_triggered = false;
    VoidService::add_request_slot(out(m_protocol_server.get_slots()),
      [&] (auto&&... args) {
        return on_void_request(std::forward<decltype(args)>(args)...,
          &callback_triggered);
      });
    m_client_protocol.send_request<VoidService>(123);
    REQUIRE(callback_triggered);
  }

  TEST_CASE_FIXTURE(Fixture, "exception") {
    IdentityService::add_request_slot(
      out(m_protocol_server.get_slots()), &on_identity_request);
    REQUIRE_THROWS_WITH_AS(m_client_protocol.send_request<IdentityService>(0),
      "Exception.", ServiceRequestException);
    auto result = m_client_protocol.send_request<IdentityService>(123);
    REQUIRE(result == 123);
  }
}
