#include <doctest/doctest.h>
#include <sstream>
#include "Beam/IO/BasicIStreamReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BasicIStreamReader") {
  TEST_CASE("create_empty") {
    auto stream = std::istringstream("");
    auto reader = BasicIStreamReader(&stream);
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read_all") {
    auto message = std::string("hello world");
    auto stream = std::istringstream(message);
    auto reader = BasicIStreamReader(&stream);
    auto buffer = SharedBuffer();
    auto size_read = reader.read(out(buffer));
    REQUIRE(size_read == message.size());
    REQUIRE(buffer == message);
  }

  TEST_CASE("read_some") {
    auto message = std::string("hello world");
    auto stream = std::istringstream(message);
    auto reader = BasicIStreamReader(&stream);
    auto buffer = SharedBuffer();
    auto size_read = reader.read(out(buffer), 6);
    REQUIRE(size_read == 6);
    REQUIRE(buffer == "hello ");
    reset(buffer);
    size_read = reader.read(out(buffer), 5);
    REQUIRE(size_read == 5);
    REQUIRE(buffer == "world");
  }

  TEST_CASE("poll") {
    auto message = std::string("abc");
    auto stream = std::istringstream(message);
    auto reader = BasicIStreamReader(&stream);
    REQUIRE(reader.poll());
    auto data = SharedBuffer();
    auto size_read = reader.read(out(data));
    REQUIRE(size_read == 3);
    REQUIRE(reader.poll() == false);
  }

  TEST_CASE("end_of_file_after_consumed") {
    auto message = std::string("xy");
    auto stream = std::istringstream(message);
    auto reader = BasicIStreamReader(&stream);
    auto data = SharedBuffer();
    auto size_read = reader.read(out(data));
    REQUIRE(size_read == 2);
    REQUIRE_THROWS_AS(reader.read(out(data), 1), EndOfFileException);
  }

  TEST_CASE("read_more_than_available_with_size") {
    auto message = std::string("short");
    auto stream = std::istringstream(message);
    auto reader = BasicIStreamReader(&stream);
    auto buffer = SharedBuffer();
    auto size_requested = std::size_t(100);
    auto size_read = reader.read(out(buffer), size_requested);
    REQUIRE(size_read == message.size());
    REQUIRE(buffer == message);
    REQUIRE_THROWS_AS(reader.read(out(buffer), 1), EndOfFileException);
  }
}
