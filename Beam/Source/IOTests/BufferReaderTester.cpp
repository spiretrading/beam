#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace boost;

TEST_SUITE("BufferReader") {
  TEST_CASE("create_empty") {
    auto reader = BufferReader<SharedBuffer>(
      BufferFromString<SharedBuffer>(""));
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.Read(Store(buffer)), EndOfFileException);
  }

  TEST_CASE("read") {
    auto message = std::string("hello world");
    auto reader = BufferReader<SharedBuffer>(
      BufferFromString<SharedBuffer>(message));
    auto data = SharedBuffer();
    auto sizeRead = reader.Read(Store(data));
    REQUIRE(message.size() == sizeRead);
    REQUIRE(data == message);
  }

  TEST_CASE("read_some_to_buffer") {
    auto message = std::string("hello world");
    auto reader = BufferReader<SharedBuffer>(
      BufferFromString<SharedBuffer>(message));
    auto data = SharedBuffer();
    auto sizeRead = reader.Read(Store(data), 6);
    REQUIRE(sizeRead == 6);
    REQUIRE(data == "hello ");
    data.Reset();
    sizeRead = reader.Read(Store(data), 5);
    REQUIRE(sizeRead == 5);
    REQUIRE(data == "world");
  }

  TEST_CASE("read_some_to_pointer") {
    auto message = std::string("hello world");
    auto reader = BufferReader<SharedBuffer>(
      BufferFromString<SharedBuffer>(message));
    auto data = std::make_unique<char[]>(message.size());
    auto sizeRead = reader.Read(data.get(), 6);
    REQUIRE(sizeRead == 6);
    REQUIRE(strncmp(data.get(), "hello ", 6) == 0);
    sizeRead = reader.Read(data.get(), 5);
    REQUIRE(sizeRead == 5);
    REQUIRE(strncmp(data.get(), "world", 5) == 0);
  }
}
