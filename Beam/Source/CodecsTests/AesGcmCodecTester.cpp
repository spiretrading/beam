#include <cryptopp/hex.h>
#include <doctest/doctest.h>
#include "Beam/Codecs/AesGcmDecoder.hpp"
#include "Beam/Codecs/AesGcmEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/StaticBuffer.hpp"

using namespace Beam;

namespace {
  auto unhex(std::string_view text) {
    auto decoder = CryptoPP::HexDecoder();
    decoder.Put(
      reinterpret_cast<const CryptoPP::byte*>(text.data()), text.size());
    decoder.MessageEnd();
    auto output = SharedBuffer(decoder.MaxRetrievable());
    decoder.Get(reinterpret_cast<CryptoPP::byte*>(output.get_mutable_data()),
      output.get_size());
    return output;
  }
}

TEST_SUITE("AesGcmCodec") {
  TEST_CASE("known_vectors") {
    auto key = from<SharedBuffer>(std::string(32, '\0'));
    auto nonce = from<SharedBuffer>(std::string(12, '\0'));
    auto plaintext = SharedBuffer();
    auto expected = SharedBuffer();
    SUBCASE("empty") {
      expected = unhex("530f8afbc74536b9a963b4f1c4cb738b");
    }
    SUBCASE("block") {
      plaintext = from<SharedBuffer>(std::string(16, '\0'));
      expected = unhex(
        "cea7403d4d606b6e074ec5d3baf39d18d0d1c8a799996bf0265b98b5d48ab919");
    }
    auto encoder = AesGcmEncoder(key, nonce);
    auto encoded = SharedBuffer();
    REQUIRE(encoder.encode(plaintext, out(encoded)) == expected.get_size());
    REQUIRE(encoded == expected);
    auto decoder = AesGcmDecoder(key, nonce);
    auto decoded = from<SharedBuffer>("previous contents");
    REQUIRE(decoder.decode(expected, out(decoded)) == plaintext.get_size());
    REQUIRE(decoded == plaintext);
  }

  TEST_CASE("records") {
    auto key = from<SharedBuffer>(std::string(32, 'k'));
    auto nonce = from<SharedBuffer>(std::string(12, 'n'));
    auto encoder = Encoder(std::in_place_type<AesGcmEncoder>, key, nonce);
    auto decoder = Decoder(std::in_place_type<AesGcmDecoder>, key, nonce);
    for(auto size : {0, 1, 15, 16, 17, 16384, 17}) {
      auto plaintext = from<SharedBuffer>(std::string(size, '\0'));
      for(auto i = 0; i != size; ++i) {
        plaintext.get_mutable_data()[i] = static_cast<char>(i);
      }
      auto encoded = SharedBuffer();
      REQUIRE(encoder.encode(plaintext, out(encoded)) ==
        plaintext.get_size() + AesGcmEncoder::TAG_SIZE);
      auto decoded = SharedBuffer(20000);
      REQUIRE(decoder.decode(encoded, out(decoded)) == plaintext.get_size());
      REQUIRE(decoded == plaintext);
    }
  }

  TEST_CASE("authentication") {
    auto key = from<SharedBuffer>(std::string(32, 'k'));
    auto nonce = from<SharedBuffer>(std::string(12, 'n'));
    auto encoder = AesGcmEncoder(key, nonce);
    auto original = SharedBuffer();
    encoder.encode(from<SharedBuffer>("message"), out(original));
    auto encoded = original;
    SUBCASE("key") {
      key.get_mutable_data()[0] ^= 1;
    }
    SUBCASE("nonce") {
      nonce.get_mutable_data()[0] ^= 1;
    }
    SUBCASE("ciphertext") {
      encoded.get_mutable_data()[0] ^= 1;
    }
    SUBCASE("tag") {
      encoded.get_mutable_data()[encoded.get_size() - 1] ^= 1;
    }
    SUBCASE("truncated_tag") {
      encoded.shrink(1);
    }
    SUBCASE("short_record") {
      encoded.shrink(encoded.get_size() - 15);
    }
    SUBCASE("trailing_data") {
      append(encoded, "extra");
    }
    SUBCASE("oversized_record") {
      reserve(encoded, AesGcmEncoder::MAXIMUM_SIZE + 17);
    }
    auto decoder = AesGcmDecoder(key, nonce);
    auto decoded = from<SharedBuffer>("unchanged");
    REQUIRE_THROWS_AS(decoder.decode(encoded, out(decoded)), DecoderException);
    REQUIRE(decoded == "unchanged");
    REQUIRE_THROWS_AS(decoder.decode(original, out(decoded)), DecoderException);
  }

  TEST_CASE("sequence") {
    auto key = from<SharedBuffer>(std::string(32, 'k'));
    auto nonce = from<SharedBuffer>(std::string(12, 'n'));
    auto encoder = AesGcmEncoder(key, nonce);
    auto first = SharedBuffer();
    auto second = SharedBuffer();
    encoder.encode(from<SharedBuffer>("same"), out(first));
    encoder.encode(from<SharedBuffer>("same"), out(second));
    REQUIRE(first != second);
    auto decoder = AesGcmDecoder(key, nonce);
    auto decoded = SharedBuffer();
    SUBCASE("replay") {
      decoder.decode(first, out(decoded));
      REQUIRE_THROWS_AS(decoder.decode(first, out(decoded)), DecoderException);
    }
    SUBCASE("out_of_order") {
      REQUIRE_THROWS_AS(decoder.decode(second, out(decoded)), DecoderException);
    }
  }

  TEST_CASE("nonce_sequence") {
    auto key = from<SharedBuffer>(std::string(32, '\0'));
    auto nonce = from<SharedBuffer>(std::string(12, '\0'));
    auto encoder = AesGcmEncoder(key, nonce);
    auto decoder = AesGcmDecoder(key, nonce);
    auto message = from<SharedBuffer>(std::string(16, '\0'));
    auto encoded = SharedBuffer();
    auto decoded = SharedBuffer();
    for(auto i = 0; i != 257; ++i) {
      encoder.encode(message, out(encoded));
      REQUIRE(decoder.decode(encoded, out(decoded)) == 16);
      REQUIRE(decoded == message);
    }
    REQUIRE(encoded == unhex(
      "9aa258269957f0ffb9eb77e38c5a836b05e55dc3f2cec73572f585068e70e877"));
  }

  TEST_CASE("destination") {
    auto key = from<SharedBuffer>(std::string(32, 'k'));
    auto nonce = from<SharedBuffer>(std::string(12, 'n'));
    auto encoder = AesGcmEncoder(key, nonce);
    auto message = from<SharedBuffer>("hello");
    auto small = StaticBuffer<20>();
    REQUIRE_THROWS_AS(encoder.encode(message, out(small)), EncoderException);
    auto encoded = StaticBuffer<21>();
    REQUIRE(encoder.encode(message, out(encoded)) == 21);
    auto decoder = AesGcmDecoder(key, nonce);
    auto insufficient = StaticBuffer<4>();
    REQUIRE_THROWS_AS(
      decoder.decode(encoded, out(insufficient)), DecoderException);
    auto decoded = StaticBuffer<5>();
    REQUIRE(decoder.decode(encoded, out(decoded)) == 5);
    REQUIRE(decoded == message);
  }

  TEST_CASE("parameters") {
    auto key = from<SharedBuffer>(std::string(32, 'k'));
    auto nonce = from<SharedBuffer>(std::string(12, 'n'));
    SUBCASE("key") {
      key.shrink(1);
    }
    SUBCASE("nonce") {
      nonce.shrink(1);
    }
    REQUIRE_THROWS_AS(AesGcmEncoder(key, nonce), EncoderException);
    REQUIRE_THROWS_AS(AesGcmDecoder(key, nonce), DecoderException);
  }

  TEST_CASE("maximum_record_size") {
    auto key = from<SharedBuffer>(std::string(32, 'k'));
    auto nonce = from<SharedBuffer>(std::string(12, 'n'));
    auto encoder = AesGcmEncoder(key, nonce);
    auto encoded = SharedBuffer();
    auto message =
      from<SharedBuffer>(std::string(AesGcmEncoder::MAXIMUM_SIZE + 1, 'x'));
    REQUIRE_THROWS_AS(encoder.encode(message, out(encoded)), EncoderException);
    REQUIRE(is_empty(encoded));
  }
}
