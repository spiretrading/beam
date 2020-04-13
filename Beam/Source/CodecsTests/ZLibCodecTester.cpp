#include <doctest/doctest.h>
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;

TEST_SUITE("ZLibCodec") {
  TEST_CASE("empty_message") {
    auto encoder = ZLibEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    auto decoder = ZLibDecoder();
    auto decodedBuffer = SharedBuffer();
    auto decodedSize = decoder.Decode(encodedBuffer, Store(decodedBuffer));
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("simple_message") {
    auto encoder = ZLibEncoder();
    auto message = BufferFromString<SharedBuffer>("hello world");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    auto decoder = ZLibDecoder();
    auto decodedBuffer = SharedBuffer();
    auto decodedSize = decoder.Decode(encodedBuffer, Store(decodedBuffer));
    REQUIRE(decodedBuffer == message);
  }
}
