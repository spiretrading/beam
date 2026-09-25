#include <algorithm>
#include <random>
#include <doctest/doctest.h>
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/StaticBuffer.hpp"

using namespace Beam;

namespace {
  struct RecordingBuffer : SharedBuffer {
    std::size_t m_maximum_size = 0;

    std::size_t grow(std::size_t size) {
      m_maximum_size = std::max(m_maximum_size, get_size() + size);
      return SharedBuffer::grow(size);
    }
  };
}

TEST_SUITE("ZLibCodec") {
  TEST_CASE("empty_message") {
    auto encoder = ZLibEncoder();
    auto message = from<SharedBuffer>("");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    auto decoder = ZLibDecoder();
    auto decoded_buffer = SharedBuffer();
    auto decoded_size = decoder.decode(encoded_buffer, out(decoded_buffer));
    REQUIRE(decoded_buffer == message);
  }

  TEST_CASE("simple_message") {
    auto encoder = ZLibEncoder();
    auto message = from<SharedBuffer>("hello world");
    auto encoded_buffer = SharedBuffer();
    auto encode_size = encoder.encode(message, out(encoded_buffer));
    auto decoder = ZLibDecoder();
    auto decoded_buffer = SharedBuffer();
    auto decoded_size = decoder.decode(encoded_buffer, out(decoded_buffer));
    REQUIRE(decoded_buffer == message);
  }

  TEST_CASE("high_compression") {
    auto text = std::string(1024 * 1024, 'x');
    auto message = SharedBuffer(text.data(), text.size());
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    auto decoded = SharedBuffer();
    REQUIRE(ZLibDecoder().decode(encoded, out(decoded)) == message.get_size());
    REQUIRE(decoded == message);
  }

  TEST_CASE("oversized_destination") {
    auto message = from<SharedBuffer>("hello world");
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    auto decoded = SharedBuffer(1024 * 1024);
    REQUIRE(ZLibDecoder().decode(encoded, out(decoded)) == message.get_size());
    REQUIRE(decoded == message);
  }

  TEST_CASE("destination_growth") {
    auto generator = std::mt19937(123);
    auto message = SharedBuffer(64 * 1024);
    for(auto i = std::size_t(0); i != message.get_size(); ++i) {
      message.get_mutable_data()[i] = static_cast<char>(generator() & 0xFF);
    }
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    auto decoded = RecordingBuffer();
    auto maximum_size = 2 * message.get_size();
    SUBCASE("empty") {}
    SUBCASE("preallocated") {
      reserve(decoded, message.get_size());
      maximum_size = message.get_size();
    }
    REQUIRE(ZLibDecoder().decode(encoded, out(decoded)) == message.get_size());
    REQUIRE(decoded == message);
    REQUIRE(decoded.m_maximum_size <= maximum_size);
  }

  TEST_CASE("fixed_destination") {
    auto message = from<SharedBuffer>("hello world");
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    SUBCASE("exact_size") {
      auto decoded = StaticBuffer<11>();
      REQUIRE(
        ZLibDecoder().decode(encoded, out(decoded)) == message.get_size());
      REQUIRE(decoded == message);
    }
    SUBCASE("insufficient_space") {
      auto decoded = StaticBuffer<10>();
      REQUIRE_THROWS_AS(
        ZLibDecoder().decode(encoded, out(decoded)), DecoderException);
    }
  }

  TEST_CASE("invalid_data") {
    auto message = from<SharedBuffer>("hello world");
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    auto invalid = encoded;
    SUBCASE("truncated_header") {
      invalid.shrink(invalid.get_size() - 1);
    }
    SUBCASE("truncated_payload") {
      invalid.shrink(invalid.get_size() / 2);
    }
    SUBCASE("truncated_checksum") {
      invalid.shrink(1);
    }
    SUBCASE("invalid_checksum") {
      invalid.get_mutable_data()[invalid.get_size() - 1] ^= 1;
    }
    SUBCASE("trailing_bytes") {
      append(invalid, from<SharedBuffer>("trailing bytes"));
    }
    SUBCASE("concatenated_streams") {
      append(invalid, encoded);
    }
    auto decoder = ZLibDecoder();
    auto decoded = SharedBuffer();
    REQUIRE_THROWS_AS(decoder.decode(invalid, out(decoded)), DecoderException);
    REQUIRE(decoded.get_size() <= 10 * invalid.get_size());
    REQUIRE(decoder.decode(encoded, out(decoded)) == message.get_size());
    REQUIRE(decoded == message);
  }

  TEST_CASE("error_diagnostics") {
    auto message = from<SharedBuffer>("hello world");
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    encoded.get_mutable_data()[encoded.get_size() - 1] ^= 1;
    auto decoded = SharedBuffer();
    auto report = [&] {
      try {
        ZLibDecoder().decode(encoded, out(decoded));
      } catch(const DecoderException& e) {
        return std::string(e.what());
      }
      return std::string();
    }();
    REQUIRE(report.contains("zlib_result=" + std::to_string(Z_DATA_ERROR)));
    REQUIRE(report.contains("zlib_message="));
    REQUIRE(report.contains(
      "compressed_size=" + std::to_string(encoded.get_size())));
    REQUIRE(report.contains("consumed="));
    REQUIRE(report.contains("produced=11"));
  }
}
