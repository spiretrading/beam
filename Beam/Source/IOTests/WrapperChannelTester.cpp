#include <type_traits>
#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/WrapperChannel.hpp"

using namespace Beam;

TEST_SUITE("WrapperChannel") {
  TEST_CASE("no_components") {
    using W = WrapperChannel<NullChannel>;
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_identifier()),
      const NamedChannelIdentifier&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_connection()),
      NullConnection&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_reader()),
      NullReader&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_writer()),
      NullWriter&>));
  }

  TEST_CASE("override_writer") {
    using W = WrapperChannel<NullChannel, BufferWriter<SharedBuffer>>;
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_writer()),
      BufferWriter<SharedBuffer>&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_reader()),
      NullReader&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_connection()),
      NullConnection&>));
  }

  TEST_CASE("override_reader_and_writer") {
    using BR = BufferReader<SharedBuffer>;
    using BW = BufferWriter<SharedBuffer>;
    using W = WrapperChannel<
      NullChannel, BufferReader<SharedBuffer>, BufferWriter<SharedBuffer>>;
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_reader()),
      BufferReader<SharedBuffer>&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_writer()),
      BufferWriter<SharedBuffer>&>));
    REQUIRE((std::is_same_v<decltype(std::declval<W&>().get_connection()),
      NullConnection&>));
  }

  TEST_CASE("deduction") {
    auto channel = NullChannel(NamedChannelIdentifier("wrapper"));
    auto wrapper1 = WrapperChannel(&channel);
    REQUIRE((std::is_same_v<
      decltype(wrapper1), WrapperChannel<NullChannel*>>));
    auto reader = BufferReader(SharedBuffer("xy", 2));
    auto wrapper2 = WrapperChannel(&channel, reader);
    REQUIRE((std::is_same_v<decltype(wrapper2),
      WrapperChannel<NullChannel*, BufferReader<SharedBuffer>>>));
    auto buffer = SharedBuffer();
    auto wrapper3 = WrapperChannel(&channel, reader,
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    REQUIRE((std::is_same_v<decltype(wrapper3),
      WrapperChannel<NullChannel*, BufferReader<SharedBuffer>,
        std::unique_ptr<BufferWriter<SharedBuffer>>>>));
  }
}
