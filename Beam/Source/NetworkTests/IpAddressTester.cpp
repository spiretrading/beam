#include <sstream>
#include <doctest/doctest.h>
#include "Beam/Network/IpAddress.hpp"

using namespace Beam;

TEST_SUITE("IpAddressTester.cpp") {
  TEST_CASE("parser") {
    REQUIRE(
      parse<IpAddress>("192.168.0.1:1234") == IpAddress("192.168.0.1", 1234));
  }

  TEST_CASE("round_trip") {
    auto source = "127.0.0.1";
    auto value = IpAddress::string_to_int(source);
    auto result = IpAddress::int_to_string(value);
    REQUIRE(result == source);
  }

  TEST_CASE("construct_getters") {
    auto address = IpAddress("10.0.0.5", 8080);
    REQUIRE(address.get_host() == "10.0.0.5");
    REQUIRE(address.get_port() == 8080);
  }

  TEST_CASE("ostream_output") {
    auto address = IpAddress("1.2.3.4", 42);
    auto out = std::stringstream();
    out << address;
    REQUIRE(out.str() == "1.2.3.4:42");
  }

  TEST_CASE("zero_address") {
    auto source = "0.0.0.0";
    auto value = IpAddress::string_to_int(source);
    auto result = IpAddress::int_to_string(value);
    REQUIRE(result == source);
  }

  TEST_CASE("default_construct_equality") {
    auto a = IpAddress();
    auto b = IpAddress();
    REQUIRE(a == b);
  }
}
