#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("NullDecoder") {
  TEST_CASE("empty_decode") {
    auto decoder = NullDecoder();
    auto message = from<SharedBuffer>("");
    auto decoded_buffer = SharedBuffer();
    auto decoded_size = decoder.decode(message, out(decoded_buffer));
    REQUIRE(decoded_size == message.get_size());
    REQUIRE(decoded_buffer == message);
  }

  TEST_CASE("empty_decode_in_place") {
    auto decoder = NullDecoder();
    auto buffer = from<SharedBuffer>("");
    auto decoded_size = decoder.decode(buffer, out(buffer));
    REQUIRE(decoded_size == buffer.get_size());
    REQUIRE(buffer == SharedBuffer());
  }

  TEST_CASE("decode") {
    auto decoder = NullDecoder();
    auto message = from<SharedBuffer>("hello");
    auto decoded_buffer = SharedBuffer();
    auto decoded_size = decoder.decode(message, out(decoded_buffer));
    REQUIRE(decoded_size == message.get_size());
    REQUIRE(decoded_buffer == message);
  }

  TEST_CASE("decode_in_place") {
    auto decoder = NullDecoder();
    auto buffer = from<SharedBuffer>("hello");
    auto decoded_size = decoder.decode(buffer, out(buffer));
    REQUIRE(decoded_size == buffer.get_size());
    REQUIRE(buffer == from<SharedBuffer>("hello"));
  }
}
