#include <array>
#include <doctest/doctest.h>
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SizeDeclarativeWriter.hpp"

using namespace boost;
using namespace boost::endian;
using namespace Beam;

namespace {
  auto read_little_endian(const char* data) {
    auto value = std::uint32_t();
    std::memcpy(&value, data, sizeof(std::uint32_t));
    return little_to_native(value);
  }
}

TEST_SUITE("SizeDeclarativeWriter") {
  TEST_CASE("write_prefix") {
    auto buffer = SharedBuffer();
    auto writer = SizeDeclarativeWriter(
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(from<SharedBuffer>("hello"));
    REQUIRE(buffer.get_size() == sizeof(std::uint32_t) + 5);
    auto data = buffer.get_data();
    auto size_value = read_little_endian(data);
    REQUIRE(size_value == 5);
    REQUIRE(std::string(data + sizeof(std::uint32_t), 5) == "hello");
  }

  TEST_CASE("write_buffer") {
    auto buffer = SharedBuffer();
    auto writer = SizeDeclarativeWriter(
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(from<SharedBuffer>("world"));
    REQUIRE(buffer.get_size() == sizeof(std::uint32_t) + 5);
    auto data = buffer.get_data();
    REQUIRE(read_little_endian(data) == 5);
    REQUIRE(std::string(data + sizeof(std::uint32_t), 5) == "world");
  }

  TEST_CASE("write_empty") {
    auto buffer = SharedBuffer();
    auto writer = SizeDeclarativeWriter(
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(SharedBuffer());
    REQUIRE(buffer.get_size() == 0);
  }

  TEST_CASE("multiple_writes") {
    auto buffer = SharedBuffer();
    auto writer = SizeDeclarativeWriter(
      std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(from<SharedBuffer>("A"));
    writer.write(from<SharedBuffer>("BC"));
    auto total = buffer.get_size();
    REQUIRE(total == 2 * sizeof(std::uint32_t) + 1 + 2);
    auto data = buffer.get_data();
    auto first_size = read_little_endian(data);
    REQUIRE(first_size == 1);
    REQUIRE(std::string(data + sizeof(std::uint32_t), 1) == "A");
    auto second_position = sizeof(std::uint32_t) + 1;
    auto second_size = read_little_endian(data + second_position);
    REQUIRE(second_size == 2);
    REQUIRE(
      std::string(data + second_position + sizeof(std::uint32_t), 2) == "BC");
  }
}
