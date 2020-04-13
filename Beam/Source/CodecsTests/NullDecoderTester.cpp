#include <doctest/doctest.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;

TEST_SUITE("NullDecoder") {
  TEST_CASE("empty_decode_from_buffer_to_buffer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("empty_decode_from_buffer_to_buffer_in_place") {
    auto decoder = NullDecoder();
    auto buffer = BufferFromString<SharedBuffer>("");
    auto decodeSize = decoder.Decode(buffer, Store(buffer));
    REQUIRE(decodeSize == buffer.GetSize());
    REQUIRE(buffer == SharedBuffer());
  }

  TEST_CASE("decode_from_buffer_to_buffer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message, Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("decode_from_buffer_to_buffer_in_place") {
    auto decoder = NullDecoder();
    auto buffer = BufferFromString<SharedBuffer>("hello");
    auto decodeSize = decoder.Decode(buffer, Store(buffer));
    REQUIRE(decodeSize == buffer.GetSize());
    REQUIRE(buffer == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("empty_decode_from_buffer_to_pointer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("");
    char decodedBuffer[16] = {'\0'};
    auto decodeSize = decoder.Decode(message, decodedBuffer,
      sizeof(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("empty_decode_from_buffer_to_pointer_in_place") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto decodeSize = decoder.Decode(message, message.GetMutableData(),
      message.GetSize());
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(message == SharedBuffer());
  }

  TEST_CASE("decode_from_buffer_to_pointer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char decodedBuffer[16] = {'\0'};
    auto decodeSize = decoder.Decode(message, decodedBuffer,
      sizeof(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("decode_from_buffer_to_pointer_in_place") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodeSize = decoder.Decode(message, message.GetMutableData(),
      message.GetSize());
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(message == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("decode_from_buffer_to_smaller_pointer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char decodedBuffer[4] = {'\0'};
    REQUIRE_THROWS_AS(decoder.Decode(message, decodedBuffer,
      sizeof(decodedBuffer)), DecoderException);
  }

  TEST_CASE("empty_decode_from_pointer_to_buffer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
      Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("empty_decode_from_pointer_to_buffer_in_place") {
    auto decoder = NullDecoder();
    auto buffer = BufferFromString<SharedBuffer>("");
    auto decodeSize = decoder.Decode(buffer.GetData(), buffer.GetSize(),
      Store(buffer));
    REQUIRE(decodeSize == buffer.GetSize());
    REQUIRE(buffer == SharedBuffer());
  }

  TEST_CASE("decode_from_pointer_to_buffer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodedBuffer = SharedBuffer();
    auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
      Store(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("decode_from_pointer_to_buffer_in_place") {
    auto decoder = NullDecoder();
    auto buffer = BufferFromString<SharedBuffer>("hello");
    auto decodeSize = decoder.Decode(buffer.GetData(), buffer.GetSize(),
      Store(buffer));
    REQUIRE(decodeSize == buffer.GetSize());
    REQUIRE(buffer == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("empty_decode_from_pointer_to_pointer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("");
    char decodedBuffer[16] = {'\0'};
    auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
      decodedBuffer, sizeof(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("empty_decode_from_pointer_to_pointer_in_place") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
      message.GetMutableData(), message.GetSize());
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(message == SharedBuffer());
  }

  TEST_CASE("decode_from_pointer_to_pointer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char decodedBuffer[16] = {'\0'};
    auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
      decodedBuffer, sizeof(decodedBuffer));
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(decodedBuffer == message);
  }

  TEST_CASE("decode_from_pointer_to_pointer_in_place") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto decodeSize = decoder.Decode(message.GetData(), message.GetSize(),
      message.GetMutableData(), message.GetSize());
    REQUIRE(decodeSize == message.GetSize());
    REQUIRE(message == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("decode_from_pointer_to_smaller_pointer") {
    auto decoder = NullDecoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char decodedBuffer[4] = {'\0'};
    REQUIRE_THROWS_AS(decoder.Decode(message.GetData(), message.GetSize(),
      decodedBuffer, sizeof(decodedBuffer)), DecoderException);
  }
}
