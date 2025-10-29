#include <sstream>
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("Buffer") {
  TEST_CASE("construct_in_place") {
    auto buffer = Buffer(std::in_place_type_t<SharedBuffer>(), "hello", 5);
    REQUIRE(buffer == "hello");
  }

  TEST_CASE("construct_from_buffer_type") {
    auto shared = SharedBuffer("world", 5);
    auto buffer = Buffer(shared);
    REQUIRE(buffer == shared);
    REQUIRE(buffer == std::string_view("world"));
    REQUIRE(std::string_view("world") == buffer);
  }

  TEST_CASE("is_empty") {
    auto empty_buffer = SharedBuffer();
    REQUIRE(is_empty(empty_buffer));
    auto non_empty_buffer = SharedBuffer("data", 4);
    REQUIRE(!is_empty(non_empty_buffer));
  }

  TEST_CASE("append_and_write") {
    auto shared = SharedBuffer("world", 5);
    auto buffer = Buffer(shared);
    append(buffer, "!", 1);
    REQUIRE(buffer == "world!");
    write(buffer, 1, static_cast<std::uint32_t>(0x61626364));
    auto value = std::uint32_t();
    std::memcpy(&value, buffer.get_data() + 1, sizeof(value));
    REQUIRE(value == std::uint32_t(0x61626364));
  }

  TEST_CASE("copy_constructor_independence") {
    auto a = Buffer(std::in_place_type_t<SharedBuffer>(), "abc", 3);
    auto b = Buffer(a);
    auto data = a.get_mutable_data();
    REQUIRE(data);
    data[0] = 'X';
    REQUIRE(b == "abc");
    REQUIRE(a.get_data()[0] == 'X');
  }

  TEST_CASE("ostream_and_equality") {
    auto buffer = Buffer(std::in_place_type_t<SharedBuffer>(), "emit", 4);
    auto os = std::stringstream();
    os << buffer;
    REQUIRE(os.str() == std::string(buffer.get_data(), buffer.get_size()));
    REQUIRE(buffer == "emit");
  }

  TEST_CASE("base64_round_trip") {
    auto source =
      Buffer(std::in_place_type_t<SharedBuffer>(), "base64-test", 11);
    auto encoded = encode_base64(source);
    auto decoded = SharedBuffer();
    decode_base64(encoded, out(decoded));
    REQUIRE(decoded == source);
  }
}
