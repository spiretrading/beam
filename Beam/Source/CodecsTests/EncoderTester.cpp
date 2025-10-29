#include <doctest/doctest.h>
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("EncoderBox") {
  TEST_CASE("in_place") {
    auto encoder = Encoder(std::in_place_type<ReverseEncoder>);
    auto message = from<SharedBuffer>("hello");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    REQUIRE(encode_size == message.get_size());
    REQUIRE(encoded_buffer == "olleh");
  }

  TEST_CASE("by_value") {
    auto encoder = Encoder(ReverseEncoder());
    auto message = from<SharedBuffer>("hello");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    REQUIRE(encode_size == message.get_size());
    REQUIRE(encoded_buffer == "olleh");
  }

  TEST_CASE("by_reference") {
    auto base_encoder = ReverseEncoder();
    auto encoder = Encoder(&base_encoder);
    auto message = from<SharedBuffer>("hello");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    REQUIRE(encode_size == message.get_size());
    REQUIRE(encoded_buffer == "olleh");
  }

  TEST_CASE("by_unique") {
    auto encoder = Encoder(std::make_unique<ReverseEncoder>());
    auto message = from<SharedBuffer>("hello");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    REQUIRE(encode_size == message.get_size());
    REQUIRE(encoded_buffer == "olleh");
  }
}
