#include <doctest/doctest.h>
#include "Beam/Utilities/ToString.hpp"
#include "Beam/WebServices/HttpVersion.hpp"

using namespace Beam;

TEST_SUITE("HttpVersion") {
  TEST_CASE("construct_default") {
    auto version = HttpVersion();
    REQUIRE(version.get_major() == 1);
    REQUIRE(version.get_minor() == 1);
    REQUIRE(version == HttpVersion::version_1_1());
  }

  TEST_CASE("version_1_0") {
    auto version = HttpVersion::version_1_0();
    REQUIRE(version.get_major() == 1);
    REQUIRE(version.get_minor() == 0);
  }

  TEST_CASE("version_1_1") {
    auto version = HttpVersion::version_1_1();
    REQUIRE(version.get_major() == 1);
    REQUIRE(version.get_minor() == 1);
  }

  TEST_CASE("stream_version_1_1") {
    auto version = HttpVersion::version_1_1();
    REQUIRE(to_string(version) == "HTTP/1.1");
  }

  TEST_CASE("stream_version_1_0") {
    auto version = HttpVersion::version_1_0();
    REQUIRE(to_string(version) == "HTTP/1.0");
  }
}
