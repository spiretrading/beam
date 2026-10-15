#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

namespace {
  struct ReadOnlyBuffer {
    std::string_view m_data;

    const char* get_data() const {
      return m_data.data();
    }

    std::size_t get_size() const {
      return m_data.size();
    }
  };
}

TEST_SUITE("DatagramPacket") {
  TEST_CASE("read_only_buffer") {
    auto buffer = ReadOnlyBuffer(std::string_view("a\0b", 3));
    auto packet = DatagramPacket(buffer, IpAddress("127.0.0.1", 80));
    auto expected = std::string("127.0.0.1:80:") + std::string("a\0b", 3);
    REQUIRE(to_string(packet) == expected);
    REQUIRE(std::format("{}", packet) == expected);
    REQUIRE(std::format("{}", buffer) == std::string("a\0b", 3));
    auto empty = DatagramPacket(ReadOnlyBuffer(), IpAddress("127.0.0.1", 80));
    REQUIRE(to_string(empty) == "127.0.0.1:80:");
    REQUIRE(std::format("{}", empty) == "127.0.0.1:80:");
  }

  TEST_CASE("format") {
    auto packet =
      DatagramPacket(SharedBuffer("a\0b", 3), IpAddress("127.0.0.1", 80));
    auto expected = std::string("127.0.0.1:80:") + std::string("a\0b", 3);
    REQUIRE(std::format("{}", packet) == expected);
    REQUIRE(
      std::format("{:*>{}}", packet, expected.size() + 2) == "**" + expected);
    REQUIRE(std::format("{:.9}", packet) == "127.0.0.1");
    REQUIRE(to_string(packet) == expected);
    REQUIRE(std::format(
      "{}", DatagramPacket(SharedBuffer(), IpAddress("127.0.0.1", 80))) ==
      "127.0.0.1:80:");
  }

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
