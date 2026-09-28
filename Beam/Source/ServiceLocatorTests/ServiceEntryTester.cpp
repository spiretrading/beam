#include <doctest/doctest.h>
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ServiceEntry") {
  TEST_CASE("constructor") {
    auto properties = JsonObject();
    properties.set("host", "192.168.1.100");
    properties.set("port", 8080);
    auto account = DirectoryEntry::make_account(123, "service_account");
    auto entry = ServiceEntry("test_service", properties, 456, account);
    REQUIRE(entry.get_name() == "test_service");
    REQUIRE(entry.get_id() == 456);
    REQUIRE(entry.get_account() == account);
    REQUIRE(entry.get_properties() == properties);
  }

  TEST_CASE("stream") {
    auto properties = JsonObject();
    properties.set("host", "localhost");
    properties.set("port", 8080);
    auto account = DirectoryEntry::make_account(42, "admin");
    auto entry = ServiceEntry("web_service", properties, 100, account);
    REQUIRE((to_string(entry) ==
      "(web_service 100 (ACCOUNT 42 admin) "
      "{\"host\":\"localhost\",\"port\":8080})" || to_string(entry) ==
      "(web_service 100 (ACCOUNT 42 admin) "
      "{\"port\":8080,\"host\":\"localhost\"})"));
    test_round_trip_shuttle(entry);
  }
}
