#include <array>
#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/QueuedReader.hpp"

using namespace Beam;

namespace {
  struct DatagramReader {
    std::array<const char*, 2> m_buffers;
    std::size_t m_count = 0;

    bool poll() const {
      return m_count != m_buffers.size();
    }

    template<IsBuffer B>
    std::size_t read(Out<B> destination) {
      return read(destination, std::size_t(65535));
    }

    template<IsBuffer B>
    std::size_t read(Out<B> destination, std::size_t size) {
      if(!poll()) {
        throw EndOfFileException();
      }
      auto capacity = destination->grow(size);
      auto character = static_cast<char>('A' + m_count);
      destination->write(0, &character, sizeof(character));
      destination->shrink(capacity - sizeof(character));
      m_buffers[m_count++] = destination->get_data();
      return sizeof(character);
    }
  };
}

TEST_SUITE("QueuedReader") {
  TEST_CASE("receive_buffer_reuse") {
    auto source = DatagramReader();
    auto reader = QueuedReader(&source);
    reader.poll();
    flush_pending_routines();
    REQUIRE(source.m_count == source.m_buffers.size());
    REQUIRE(source.m_buffers[0] == source.m_buffers[1]);
    auto buffer = SharedBuffer();
    REQUIRE(reader.read(out(buffer)) == 1);
    REQUIRE(buffer == "A");
    reset(buffer);
    REQUIRE(reader.read(out(buffer)) == 1);
    REQUIRE(buffer == "B");
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read") {
    auto reader = QueuedReader(BufferReader(from<SharedBuffer>("world")));
    auto buffer = SharedBuffer();
    REQUIRE(reader.read(out(buffer)) == 5);
    REQUIRE(buffer == "world");
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read_partial") {
    auto reader = QueuedReader(BufferReader(from<SharedBuffer>("abcde")));
    auto part1 = SharedBuffer();
    auto read1 = reader.read(out(part1), 2);
    REQUIRE(read1 == 2);
    REQUIRE(part1 == "ab");
    auto part2 = SharedBuffer();
    auto read2 = reader.read(out(part2), 3);
    REQUIRE(read2 == 3);
    REQUIRE(part2 == "cde");
    REQUIRE_THROWS_AS(reader.read(out(part2), 1), EndOfFileException);
  }
}
