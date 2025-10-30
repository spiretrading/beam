#include <doctest/doctest.h>
#include "Beam/Utilities/ToString.hpp"
#include "Beam/WebServices/HttpHeader.hpp"

using namespace Beam;

TEST_SUITE("HttpHeader") {
  TEST_CASE("construct_with_name_and_value") {
    auto header = HttpHeader("Content-Type", "application/json");
    REQUIRE(header.get_name() == "Content-Type");
    REQUIRE(header.get_value() == "application/json");
  }

  TEST_CASE("construct_with_empty_strings") {
    auto header = HttpHeader("", "");
    REQUIRE(header.get_name() == "");
    REQUIRE(header.get_value() == "");
  }

  TEST_CASE("stream") {
    auto header = HttpHeader("Host", "example.com");
    REQUIRE(to_string(header) == "Host: example.com");
  }

  TEST_CASE("output_stream_operator_with_empty_value") {
    auto header = HttpHeader("X-Custom-Header", "");
    REQUIRE(to_string(header) == "X-Custom-Header: ");
  }

  TEST_CASE("output_stream_operator_with_empty_name") {
    auto header = HttpHeader("", "some value");
    REQUIRE(to_string(header) == ": some value");
  }

  TEST_CASE("output_stream_operator_with_spaces_in_value") {
    auto header = HttpHeader(
      "Content-Disposition", "attachment; filename=\"test file.pdf\"");
    REQUIRE(to_string(header) ==
      "Content-Disposition: attachment; filename=\"test file.pdf\"");
  }

  TEST_CASE("output_stream_operator_with_special_characters") {
    auto header = HttpHeader("Set-Cookie", "sessionId=abc123; Path=/; Secure");
    REQUIRE(
      to_string(header) == "Set-Cookie: sessionId=abc123; Path=/; Secure");
  }
}
