#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BufferReader") {
  TEST_CASE("shared_buffer_transfer") {
    auto source = from<SharedBuffer>("abcdef");
    auto reader = BufferReader(source);
    auto offset = std::size_t(0);
    SUBCASE("whole") {}
    SUBCASE("remainder") {
      auto prefix = SharedBuffer();
      REQUIRE(reader.read(out(prefix), 2) == 2);
      REQUIRE(prefix == "ab");
      offset = 2;
    }
    auto destination = SharedBuffer();
    REQUIRE(reader.read(out(destination)) == source.get_size() - offset);
    REQUIRE(destination.get_data() == source.get_data() + offset);
    REQUIRE_FALSE(reader.poll());
    REQUIRE_THROWS_AS(reader.read(out(destination)), EndOfFileException);
    reader = BufferReader(SharedBuffer());
    source = SharedBuffer();
    REQUIRE(destination == std::string_view("abcdef").substr(offset));
    append(destination, "gh", 2);
    REQUIRE(destination == std::string_view("abcdefgh").substr(offset));
  }

  TEST_CASE("create_empty") {
    auto reader = BufferReader(from<SharedBuffer>(""));
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read_all") {
    auto message = std::string("hello world");
    auto reader = BufferReader(from<SharedBuffer>(message));
    auto buffer = SharedBuffer();
    auto size_read = reader.read(out(buffer));
    REQUIRE(size_read == message.size());
    REQUIRE(buffer == message);
  }

  TEST_CASE("read_some_to_buffer") {
    auto reader = BufferReader(from<SharedBuffer>("hello world"));
    auto buffer = SharedBuffer();
    auto size_read = reader.read(out(buffer), 6);
    REQUIRE(size_read == 6);
    REQUIRE(buffer == "hello ");
    reset(buffer);
    size_read = reader.read(out(buffer), 5);
    REQUIRE(size_read == 5);
    REQUIRE(buffer == "world");
  }

  TEST_CASE("poll_behavior") {
    auto reader = BufferReader(from<SharedBuffer>("abc"));
    REQUIRE(reader.poll() == true);
    auto data = SharedBuffer();
    auto size_read = reader.read(out(data), 3);
    REQUIRE(size_read == 3);
    REQUIRE(!reader.poll());
  }

  TEST_CASE("copy_independence") {
    auto reader = BufferReader(from<SharedBuffer>("abcdefgh"));
    auto reader_copy = reader;
    auto a = SharedBuffer();
    auto b = SharedBuffer();
    auto a_read = reader.read(out(a), 3);
    REQUIRE(a_read == 3);
    auto b_read = reader_copy.read(out(b), 4);
    REQUIRE(b_read == 4);
    REQUIRE(a == std::string("abc"));
    REQUIRE(b == std::string("abcd"));
    REQUIRE(reader.poll());
    REQUIRE(reader_copy.poll());
  }

  TEST_CASE("shared_buffer_mutation") {
    auto reader = BufferReader(from<SharedBuffer>("abcdef"));
    auto copy = reader;
    auto destination = SharedBuffer();
    REQUIRE(reader.read(out(destination)) == 6);
    destination.write(0, "X", 1);
    auto original = SharedBuffer();
    REQUIRE(copy.read(out(original)) == 6);
    REQUIRE(original == "abcdef");
    REQUIRE(destination == "Xbcdef");
  }

  TEST_CASE("end_of_file_after_consumed") {
    auto reader = BufferReader(from<SharedBuffer>("xy"));
    auto data = SharedBuffer();
    auto size_read = reader.read(out(data));
    REQUIRE(size_read == 2);
    REQUIRE_THROWS_AS(reader.read(out(data), 1), EndOfFileException);
  }
}
