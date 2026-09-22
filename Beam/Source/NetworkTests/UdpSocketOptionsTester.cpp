#include <doctest/doctest.h>
#include "Beam/Network/MulticastSocketOptions.hpp"

using namespace Beam;

TEST_SUITE("UdpSocketOptions") {
  TEST_CASE("loopback") {
    auto options = MulticastSocketOptions();
    REQUIRE(options.m_enable_loopback);
    REQUIRE(UdpSocketOptions().m_enable_loopback);
  }
}
