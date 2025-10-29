#include <doctest/doctest.h>
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("Channel") {
  TEST_CASE("construct_components") {
    auto write_buffer = SharedBuffer();
    auto channel = Channel(std::in_place_type<BasicChannel<
      NamedChannelIdentifier, NullConnection, BufferReader<SharedBuffer>,
      std::unique_ptr<BufferWriter<SharedBuffer>>>>,
      NamedChannelIdentifier("channel"), NullConnection(),
      BufferReader(from<SharedBuffer>("hello")),
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(write_buffer)));
    REQUIRE(to_string(channel.get_identifier()) == "channel");
    auto read_buffer = SharedBuffer();
    auto read_count = channel.get_reader().read(out(read_buffer));
    REQUIRE(read_buffer == "hello");
    channel.get_writer().write(from<SharedBuffer>("world"));
    REQUIRE(write_buffer == "world");
    REQUIRE_NOTHROW(channel.get_connection().close());
  }
}
