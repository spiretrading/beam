#include <limits>
#include <doctest/doctest.h>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/StaticBuffer.hpp"

using namespace boost;
using namespace boost::endian;
using namespace Beam;
using namespace Beam::Tests;

namespace {
  struct RecordingBuffer : StaticBuffer<1024> {
    std::size_t m_maximum_size = 0;

    std::size_t grow(std::size_t size) {
      m_maximum_size = std::max(m_maximum_size, get_size() + size);
      return StaticBuffer<1024>::grow(size);
    }
  };
}

TEST_SUITE("SizeDeclarativeDecoder") {
  TEST_CASE("empty_decode") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = from<SharedBuffer>("");
    auto decoded_message = from<SharedBuffer>("");
    auto output = SharedBuffer();
    append(output, native_to_big<std::uint32_t>(message.get_size()));
    append(output, message);
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(output, out(decoded_buffer));
    auto expected_decode_size = decoded_message.get_size();
    REQUIRE(decode_size == expected_decode_size);
    REQUIRE(decoded_buffer == decoded_message);
  }

  TEST_CASE("decode") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = from<SharedBuffer>("hello");
    auto decoded_message = from<SharedBuffer>("olleh");
    auto output = SharedBuffer();
    append(output, native_to_big<std::uint32_t>(message.get_size()));
    append(output, message);
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(output, out(decoded_buffer));
    auto expected_decode_size = decoded_message.get_size();
    REQUIRE(decode_size == expected_decode_size);
    REQUIRE(decoded_buffer == decoded_message);
  }

  TEST_CASE("size_mismatch") {
    auto declared_size = std::uint32_t();
    SUBCASE("smaller") {
      declared_size = 4;
    }
    SUBCASE("larger") {
      declared_size = 6;
    }
    auto encoded = SharedBuffer();
    append(encoded, native_to_big(declared_size));
    append(encoded, "hello", 5);
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto decoded = SharedBuffer();
    REQUIRE_THROWS_AS(decoder.decode(encoded, out(decoded)), DecoderException);
  }

  TEST_CASE("excessive_size_declaration") {
    auto payload = SharedBuffer();
    ZLibEncoder().encode(from<SharedBuffer>("hello"), out(payload));
    auto encoded = SharedBuffer();
    append(encoded, native_to_big(std::numeric_limits<std::uint32_t>::max()));
    append(encoded, payload);
    auto decoder = SizeDeclarativeDecoder<ZLibDecoder>();
    auto decoded = RecordingBuffer();
    REQUIRE_THROWS_AS(decoder.decode(encoded, out(decoded)), DecoderException);
    constexpr auto MAXIMUM_RESERVATION = std::size_t(1024 * 1024);
    REQUIRE(decoded.m_maximum_size <= MAXIMUM_RESERVATION);
  }

  TEST_CASE("large_compressed_message") {
    auto message = from<SharedBuffer>(std::string(2 * 1024 * 1024, 'x'));
    auto encoder = SizeDeclarativeEncoder<ZLibEncoder>();
    auto encoded = SharedBuffer();
    encoder.encode(message, out(encoded));
    auto decoder = SizeDeclarativeDecoder<ZLibDecoder>();
    auto decoded = SharedBuffer();
    REQUIRE(decoder.decode(encoded, out(decoded)) == message.get_size());
    REQUIRE(decoded == message);
  }
}
