#include <doctest/doctest.h>
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("ZLibCodec") {
  TEST_CASE("empty_message") {
    auto encoder = ZLibEncoder();
    auto message = from<SharedBuffer>("");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    auto decoder = ZLibDecoder();
    auto decoded_buffer = SharedBuffer();
    auto decoded_size = decoder.decode(encoded_buffer, out(decoded_buffer));
    REQUIRE(decoded_buffer == message);
  }

  TEST_CASE("simple_message") {
    auto encoder = ZLibEncoder();
    auto message = from<SharedBuffer>("hello world");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    auto decoder = ZLibDecoder();
    auto decoded_buffer = SharedBuffer();
    auto decoded_size = decoder.decode(encoded_buffer, out(decoded_buffer));
    REQUIRE(decoded_buffer == message);
  }
}
