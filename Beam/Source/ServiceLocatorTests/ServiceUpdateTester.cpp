#include <doctest/doctest.h>
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/ServiceLocator/ServiceUpdate.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ServiceUpdate") {
  TEST_CASE("add_and_remove") {
    auto properties = JsonObject();
    properties.set("scope", "TSX");
    auto service = ServiceEntry("quotes", properties, 42,
      DirectoryEntry::make_account(12, "provider"));
    auto addition = ServiceUpdate::add(service);
    REQUIRE(addition.m_service == service);
    REQUIRE(addition.m_type == ServiceUpdate::Type::ADDED);
    REQUIRE(to_string(addition) ==
      "((quotes 42 (ACCOUNT 12 provider) {\"scope\":\"TSX\"}) ADDED)");
    auto removal = ServiceUpdate::remove(service);
    REQUIRE(removal.m_service == service);
    REQUIRE(removal.m_type == ServiceUpdate::Type::REMOVED);
    REQUIRE(to_string(removal) ==
      "((quotes 42 (ACCOUNT 12 provider) {\"scope\":\"TSX\"}) REMOVED)");
    test_round_trip_shuttle(removal, [&] (const auto& received) {
      REQUIRE(received == removal);
      REQUIRE(received.m_service.get_name() == service.get_name());
      REQUIRE(received.m_service.get_properties() == properties);
      REQUIRE(received.m_service.get_account() == service.get_account());
      REQUIRE(received.m_service.get_account().m_name == "provider");
    });
  }
}
