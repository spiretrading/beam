#include <doctest/doctest.h>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  struct CountingBuffer : SharedBuffer {
    int* m_copy_count;

    CountingBuffer(SharedBuffer buffer, Ref<int> copy_count)
      : SharedBuffer(std::move(buffer)),
        m_copy_count(&*copy_count) {}

    CountingBuffer(const CountingBuffer& buffer)
      : SharedBuffer(buffer),
        m_copy_count(buffer.m_copy_count) {
      ++*m_copy_count;
    }
  };
}

TEST_SUITE("Decoder") {
  TEST_CASE("source_copies") {
    auto message = from<SharedBuffer>("hello world");
    auto encoded = SharedBuffer();
    ZLibEncoder().encode(message, out(encoded));
    auto copy_count = 0;
    auto decoded = SharedBuffer();
    SUBCASE("zlib") {
      auto source = CountingBuffer(encoded, Ref(copy_count));
      REQUIRE(ZLibDecoder().decode(source, out(decoded)) == message.get_size());
    }
    SUBCASE("type_erased") {
      auto source = CountingBuffer(encoded, Ref(copy_count));
      auto decoder = Decoder(ZLibDecoder());
      REQUIRE(decoder.decode(source, out(decoded)) == message.get_size());
    }
    SUBCASE("size_declarative") {
      reset(encoded);
      SizeDeclarativeEncoder<ZLibEncoder>().encode(message, out(encoded));
      auto source = CountingBuffer(encoded, Ref(copy_count));
      auto decoder = SizeDeclarativeDecoder<ZLibDecoder>();
      REQUIRE(decoder.decode(source, out(decoded)) == message.get_size());
    }
    REQUIRE(decoded == message);
    REQUIRE(copy_count == 0);
  }

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
