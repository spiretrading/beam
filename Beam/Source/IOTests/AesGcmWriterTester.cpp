#include <limits>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Codecs/AesGcmDecoder.hpp"
#include "Beam/IO/AesGcmWriter.hpp"

using namespace Beam;

namespace {
  class TestWriter {
    public:
      std::vector<SharedBuffer> m_records;
      std::size_t m_limit;

      TestWriter()
        : m_limit(std::numeric_limits<std::size_t>::max()) {}

      template<IsConstBuffer B>
      void write(const B& buffer) {
        if(m_records.size() == m_limit) {
          throw std::logic_error("Test write failure.");
        }
        m_records.emplace_back(buffer.get_data(), buffer.get_size());
      }
  };
}

TEST_SUITE("AesGcmWriter") {
  TEST_CASE("record_sizes") {
    auto key = from<SharedBuffer>(std::string(AesGcmEncoder::KEY_SIZE, '\0'));
    auto nonce =
      from<SharedBuffer>(std::string(AesGcmEncoder::NONCE_SIZE, '\0'));
    auto destination = TestWriter();
    auto writer = AesGcmWriter(&destination, key, nonce);
    auto decoder = AesGcmDecoder(key, nonce);
    auto maximum = AesGcmEncoder::MAXIMUM_SIZE;
    for(auto size : {std::size_t(0), std::size_t(1), maximum - 1, maximum,
        maximum + 1, 2 * maximum, 2 * maximum + 1}) {
      auto message = SharedBuffer(size);
      for(auto i = std::size_t(0); i != size; ++i) {
        message.get_mutable_data()[i] = static_cast<char>(i % 251);
      }
      writer.write(message);
      REQUIRE(destination.m_records.size() ==
        (size + maximum - 1) / maximum);
      auto received = SharedBuffer();
      auto offset = std::size_t(0);
      for(auto& record : destination.m_records) {
        auto count = std::min(maximum, size - offset);
        REQUIRE(record.get_size() == count + AesGcmEncoder::TAG_SIZE);
        auto plaintext = SharedBuffer();
        REQUIRE(decoder.decode(record, out(plaintext)) == count);
        append(received, plaintext);
        offset += count;
      }
      REQUIRE(received == message);
      destination.m_records.clear();
    }
  }

  TEST_CASE("write_failure") {
    auto key = from<SharedBuffer>(std::string(AesGcmEncoder::KEY_SIZE, '\0'));
    auto nonce =
      from<SharedBuffer>(std::string(AesGcmEncoder::NONCE_SIZE, '\0'));
    auto destination = TestWriter();
    destination.m_limit = 1;
    auto writer = AesGcmWriter(&destination, key, nonce);
    auto message =
      from<SharedBuffer>(std::string(3 * AesGcmEncoder::MAXIMUM_SIZE, 'x'));
    REQUIRE_THROWS_AS(writer.write(message), std::logic_error);
    REQUIRE(destination.m_records.size() == 1);
    auto decoder = AesGcmDecoder(key, nonce);
    auto plaintext = SharedBuffer();
    decoder.decode(destination.m_records.front(), out(plaintext));
    REQUIRE(plaintext == std::string(AesGcmEncoder::MAXIMUM_SIZE, 'x'));
  }
}
