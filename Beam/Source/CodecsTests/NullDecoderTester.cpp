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

  TEST_CASE("reused_destination") {
    auto destination = from<SharedBuffer>("abcdef");
    auto source = SharedBuffer();
    SUBCASE("empty") {}
    SUBCASE("shorter") {
      source = from<SharedBuffer>("hi");
    }
    SUBCASE("shared_prefix") {
      source = destination.slice(0, 2);
    }
    REQUIRE(NullDecoder().decode(source, out(destination)) == source.get_size());
    REQUIRE(destination == source);
  }

  TEST_CASE("decode_shared_prefix") {
    auto decoder = NullDecoder();
    auto source = from<SharedBuffer>("abcdef");
    auto destination = source.slice(0, 3);
    auto decoded_size = decoder.decode(source, out(destination));
    REQUIRE(decoded_size == source.get_size());
    REQUIRE(destination.get_size() == decoded_size);
    REQUIRE(destination == source);
    REQUIRE(source == "abcdef");
  }
}
