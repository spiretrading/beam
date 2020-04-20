#include <doctest/doctest.h>
#include "Beam/Network/IpAddress.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Parsers;

TEST_SUITE("IpAddress") {
  TEST_CASE("parser") {
    auto parser = IpAddressParser();
    auto source = ParserStreamFromString("192.168.0.1:1234");
    auto ipAddress = IpAddress();
    REQUIRE(parser.Read(source, ipAddress));
    REQUIRE(ipAddress == IpAddress("192.168.0.1", 1234));
  }
}
