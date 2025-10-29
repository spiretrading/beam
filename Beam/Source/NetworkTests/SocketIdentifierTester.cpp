#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Network/SocketIdentifier.hpp"

using namespace Beam;

TEST_SUITE("SocketIdentifier") {
  TEST_CASE("default_constructor") {
    auto identifier = SocketIdentifier();
    auto ss = std::stringstream();
    ss << identifier;
    REQUIRE(ss.str() == ":0");
    REQUIRE(identifier.get_address() == IpAddress());
  }

  TEST_CASE("constructor") {
    auto address = IpAddress("127.0.0.1", 8080);
    auto identifier = SocketIdentifier(std::move(address));
    auto ss = std::stringstream();
    ss << identifier;
    REQUIRE(ss.str() == "127.0.0.1:8080");
  }

  TEST_CASE("get_address") {
    auto address = IpAddress("10.1.2.3", 9999);
    auto identifier = SocketIdentifier(address);
    auto const& const_identifier = identifier;
    REQUIRE(identifier.get_address() == address);
    REQUIRE(const_identifier.get_address() == address);
  }
}
