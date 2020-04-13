#include <doctest/doctest.h>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;

TEST_SUITE("SizeDeclarativeDecoder") {
  TEST_CASE("empty_decode_from_buffer_to_buffer") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = BufferFromString<SharedBuffer>("");
    auto decodedMessage = BufferFromString<SharedBuffer>("");
    auto outputBuffer = SharedBuffer();
    outputBuffer.Append(ToBigEndian<std::uint32_t>(message.GetSize()));
    outputBuffer.Append(message);
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(outputBuffer, Store(decodedBuffer));
    auto expectedDecodeSize = decodedMessage.GetSize();
    REQUIRE(decodeSize == expectedDecodeSize);
    REQUIRE(decodedBuffer == decodedMessage);
  }

  TEST_CASE("decode_from_buffer_to_buffer") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedMessage = BufferFromString<SharedBuffer>("olleh");
    auto outputBuffer = SharedBuffer();
    outputBuffer.Append(ToBigEndian<std::uint32_t>(message.GetSize()));
    outputBuffer.Append(message);
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(outputBuffer, Store(decodedBuffer));
    auto expectedDecodeSize = decodedMessage.GetSize();
    REQUIRE(decodeSize == expectedDecodeSize);
    REQUIRE(decodedBuffer == decodedMessage);
  }

  TEST_CASE("TestEmptyDecodeFromBufferToPointer") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = BufferFromString<SharedBuffer>("");
    auto decodedMessage = BufferFromString<SharedBuffer>("");
    auto outputBuffer = SharedBuffer();
    outputBuffer.Append(ToBigEndian<std::uint32_t>(message.GetSize()));
    outputBuffer.Append(message);
    auto decodedBuffer = SharedBuffer();
    decodedBuffer.Reserve(decodedMessage.GetSize());
    auto decodeSize = decoder.Decode(outputBuffer,
      decodedMessage.GetMutableData(), decodedMessage.GetSize());
    auto expectedDecodeSize = decodedMessage.GetSize();
    REQUIRE(decodeSize == expectedDecodeSize);
    REQUIRE(decodedBuffer == decodedMessage);
  }
}
