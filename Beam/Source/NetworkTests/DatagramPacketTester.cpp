#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("DatagramPacket") {
  TEST_CASE("default_construct") {
    auto packet = DatagramPacket<SharedBuffer>();
    REQUIRE(packet.get_data().get_size() == 0);
  }

  TEST_CASE("construct_with_buffer_and_address") {
    auto buffer = from<SharedBuffer>("payload");
    auto address = IpAddress("127.0.0.1", 9999);
    auto packet = DatagramPacket(std::move(buffer), address);
    REQUIRE(packet.get_data() == "payload");
    REQUIRE(packet.get_address() == address);
  }

  TEST_CASE("ostream_output") {
    auto buffer = from<SharedBuffer>("payload");
    auto address = IpAddress("127.0.0.1", 9999);
    auto packet = DatagramPacket(std::move(buffer), address);
    REQUIRE(to_string(packet) == "127.0.0.1:9999:payload");
  }

  TEST_CASE("empty_payload_streaming") {
    auto buffer = SharedBuffer();
    auto address = IpAddress("127.0.0.1", 80);
    auto packet = DatagramPacket(std::move(buffer), address);
    REQUIRE(to_string(packet) == "127.0.0.1:80:");
  }
}
