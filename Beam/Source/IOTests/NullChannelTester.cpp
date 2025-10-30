#include <doctest/doctest.h>
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

TEST_SUITE("NullChannel") {
  TEST_CASE("default_constructor") {
    auto channel = NullChannel();
    REQUIRE(to_string(channel.get_identifier()).empty());
  }

  TEST_CASE("constructor") {
    auto channel = NullChannel(NamedChannelIdentifier("null-id"));
    REQUIRE(to_string(channel.get_identifier()) == "null-id");
  }

  TEST_CASE("getters") {
    auto channel = NullChannel();
    REQUIRE_NOTHROW(channel.get_connection().close());
    REQUIRE(!channel.get_reader().poll());
    auto data = SharedBuffer();
    REQUIRE_THROWS_AS(
      channel.get_reader().read(out(data), 1), EndOfFileException);
    REQUIRE_NOTHROW(channel.get_writer().write(SharedBuffer()));
  }
}
