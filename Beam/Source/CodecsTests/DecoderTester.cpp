#include <doctest/doctest.h>
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("Decoder") {
  TEST_CASE("in_place") {
    auto decoder = Decoder(std::in_place_type<ReverseDecoder>);
    auto message = from<SharedBuffer>("hello");
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(message, out(decoded_buffer));
    REQUIRE(decode_size == message.get_size());
    REQUIRE(decoded_buffer == "olleh");
  }

  TEST_CASE("by_value") {
    auto decoder = Decoder(ReverseDecoder());
    auto message = from<SharedBuffer>("hello");
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(message, out(decoded_buffer));
    REQUIRE(decode_size == message.get_size());
    REQUIRE(decoded_buffer == "olleh");
  }

  TEST_CASE("by_reference") {
    auto base_decoder = ReverseDecoder();
    auto decoder = Decoder(&base_decoder);
    auto message = from<SharedBuffer>("hello");
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(message, out(decoded_buffer));
    REQUIRE(decode_size == message.get_size());
    REQUIRE(decoded_buffer == "olleh");
  }

  TEST_CASE("by_unique") {
    auto decoder = Decoder(std::make_unique<ReverseDecoder>());
    auto message = from<SharedBuffer>("hello");
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(message, out(decoded_buffer));
    REQUIRE(decode_size == message.get_size());
    REQUIRE(decoded_buffer == "olleh");
  }
}
