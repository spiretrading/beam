#include <doctest/doctest.h>
#include "Beam/Codecs/DecoderBox.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;

TEST_SUITE("DecoderBox") {
  TEST_CASE("in_place") {
    auto decoder = DecoderBox(std::in_place_type<ReverseDecoder>);
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == "olleh");
  }

  TEST_CASE("by_value") {
    auto decoder = DecoderBox(ReverseDecoder());
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == "olleh");
  }

  TEST_CASE("by_reference") {
    auto baseDecoder = ReverseDecoder();
    auto decoder = DecoderBox(&baseDecoder);
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == "olleh");
  }

  TEST_CASE("by_unique") {
    auto decoder = DecoderBox(std::make_unique<ReverseDecoder>());
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == "olleh");
  }
}
