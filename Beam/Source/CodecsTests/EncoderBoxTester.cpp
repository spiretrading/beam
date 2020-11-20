#include <doctest/doctest.h>
#include "Beam/Codecs/EncoderBox.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;

TEST_SUITE("EncoderBox") {
  TEST_CASE("in_place") {
    auto encoder = EncoderBox(std::in_place_type<ReverseEncoder>);
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == "olleh");
  }

  TEST_CASE("by_value") {
    auto encoder = EncoderBox(ReverseEncoder());
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == "olleh");
  }

  TEST_CASE("by_reference") {
    auto baseEncoder = ReverseEncoder();
    auto encoder = EncoderBox(&baseEncoder);
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == "olleh");
  }

  TEST_CASE("by_unique") {
    auto encoder = EncoderBox(std::make_unique<ReverseEncoder>());
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == "olleh");
  }
}
