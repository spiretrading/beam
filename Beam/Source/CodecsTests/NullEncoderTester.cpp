#include <doctest/doctest.h>
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;

TEST_SUITE("NullEncoder") {
  TEST_CASE("empty_encode_from_buffer_to_buffer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("empty_encode_from_buffer_to_buffer_in_place") {
    auto encoder = NullEncoder();
    auto buffer = BufferFromString<SharedBuffer>("");
    auto encodeSize = encoder.Encode(buffer, Store(buffer));
    REQUIRE(encodeSize == buffer.GetSize());
    REQUIRE(buffer == SharedBuffer());
  }

  TEST_CASE("encode_from_buffer_to_buffer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message, Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("encode_from_buffer_to_buffer_in_place") {
    auto encoder = NullEncoder();
    auto buffer = BufferFromString<SharedBuffer>("hello");
    auto encodeSize = encoder.Encode(buffer, Store(buffer));
    REQUIRE(encodeSize == buffer.GetSize());
    REQUIRE(buffer == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("empty_encode_from_buffer_to_pointer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    char encodedBuffer[16] = {'\0'};
    auto encodeSize = encoder.Encode(message, encodedBuffer,
      sizeof(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("empty_encode_from_buffer_to_pointer_in_place") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto encodeSize = encoder.Encode(message, message.GetMutableData(),
      message.GetSize());
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(message == SharedBuffer());
  }

  TEST_CASE("encode_from_buffer_to_pointer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char encodedBuffer[16] = {'\0'};
    auto encodeSize = encoder.Encode(message, encodedBuffer,
      sizeof(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("encode_from_buffer_to_pointer_in_place") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodeSize = encoder.Encode(message, message.GetMutableData(),
      message.GetSize());
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(message == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("encode_from_buffer_to_smaller_pointer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char encodedBuffer[4] = {'\0'};
    REQUIRE_THROWS_AS(encoder.Encode(message, encodedBuffer,
      sizeof(encodedBuffer)), EncoderException);
  }

  TEST_CASE("empty_encode_from_pointer_to_buffer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
      Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("empty_encode_from_pointer_to_buffer_in_place") {
    auto encoder = NullEncoder();
    auto buffer = BufferFromString<SharedBuffer>("");
    auto encodeSize = encoder.Encode(buffer.GetData(), buffer.GetSize(),
      Store(buffer));
    REQUIRE(encodeSize == buffer.GetSize());
    REQUIRE(buffer == SharedBuffer());
  }

  TEST_CASE("encode_from_pointer_to_buffer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodedBuffer = SharedBuffer();
    auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
      Store(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("encode_from_pointer_to_buffer_in_place") {
    auto encoder = NullEncoder();
    auto buffer = BufferFromString<SharedBuffer>("hello");
    auto encodeSize = encoder.Encode(buffer.GetData(), buffer.GetSize(),
      Store(buffer));
    REQUIRE(encodeSize == buffer.GetSize());
    REQUIRE(buffer == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("empty_encode_from_pointer_to_pointer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    char encodedBuffer[16] = {'\0'};
    auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
      encodedBuffer, sizeof(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("empty_encode_from_pointer_to_pointer_in_place") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("");
    auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
      message.GetMutableData(), message.GetSize());
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(message == SharedBuffer());
  }

  TEST_CASE("encode_from_pointer_to_pointer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char encodedBuffer[16] = {'\0'};
    auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
      encodedBuffer, sizeof(encodedBuffer));
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(encodedBuffer == message);
  }

  TEST_CASE("encode_from_pointer_to_pointer_in_place") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    auto encodeSize = encoder.Encode(message.GetData(), message.GetSize(),
      message.GetMutableData(), message.GetSize());
    REQUIRE(encodeSize == message.GetSize());
    REQUIRE(message == BufferFromString<SharedBuffer>("hello"));
  }

  TEST_CASE("encode_from_pointer_to_smaller_pointer") {
    auto encoder = NullEncoder();
    auto message = BufferFromString<SharedBuffer>("hello");
    char encodedBuffer[4] = {'\0'};
    REQUIRE_THROWS_AS(encoder.Encode(message.GetData(), message.GetSize(),
      encodedBuffer, sizeof(encodedBuffer)), EncoderException);
  }
}
