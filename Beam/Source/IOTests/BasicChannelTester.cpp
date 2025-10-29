#include <doctest/doctest.h>
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BasicChannel") {
  TEST_CASE("construct_components") {
    auto write_buffer = SharedBuffer();
    auto channel = BasicChannel(NamedChannelIdentifier("channel"),
      NullConnection(), BufferReader(from<SharedBuffer>("hello")),
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(write_buffer)));
    auto ss = std::stringstream();
    ss << channel.get_identifier();
    REQUIRE(ss.str() == "channel");
    auto read_buffer = SharedBuffer();
    auto read_count = channel.get_reader().read(out(read_buffer));
    REQUIRE(read_buffer == "hello");
    channel.get_writer().write(from<SharedBuffer>("world"));
    REQUIRE(write_buffer == "world");
    REQUIRE_NOTHROW(channel.get_connection().close());
  }
}
