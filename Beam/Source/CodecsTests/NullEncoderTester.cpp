#include <doctest/doctest.h>
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("NullEncoder") {
  TEST_CASE("empty_encode") {
    auto encoder = NullEncoder();
    auto message = from<SharedBuffer>("");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    REQUIRE(encode_size == message.get_size());
    REQUIRE(encoded_buffer == message);
  }

  TEST_CASE("empty_encode_in_place") {
    auto encoder = NullEncoder();
    auto buffer = from<SharedBuffer>("");
    auto encode_size = encoder.encode(buffer, out(buffer));
    REQUIRE(encode_size == buffer.get_size());
    REQUIRE(buffer == SharedBuffer());
  }

  TEST_CASE("encode") {
    auto encoder = NullEncoder();
    auto message = from<SharedBuffer>("hello");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    REQUIRE(encode_size == message.get_size());
    REQUIRE(encoded_buffer == message);
  }

  TEST_CASE("encode_in_place") {
    auto encoder = NullEncoder();
    auto buffer = from<SharedBuffer>("hello");
    auto encode_size = encoder.encode(buffer, out(buffer));
    REQUIRE(encode_size == buffer.get_size());
    REQUIRE(buffer == from<SharedBuffer>("hello"));
  }
}
