#include <sstream>
#include <doctest/doctest.h>
#include "Beam/IO/BufferRef.hpp"
#include "Beam/IO/BufferView.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BufferView") {
  TEST_CASE("borrowed_range") {
    auto data = std::string("a\0bcd", 5);
    auto view = BufferView(data.data() + 1, 3);
    auto reference = BufferCRef(view);
    auto output = SharedBuffer();
    append(output, reference);
    REQUIRE(output == std::string_view("\0bc", 3));
    REQUIRE(view.get_data() == data.data() + 1);
    data[2] = 'x';
    reset(output);
    append(output, view);
    REQUIRE(output == std::string_view("\0xc", 3));
    auto stream = std::ostringstream();
    stream << view;
    REQUIRE(stream.str() == std::string("\0xc", 3));
  }

  TEST_CASE("empty_range") {
    auto data = std::string("data");
    auto pointer = static_cast<const void*>(nullptr);
    SUBCASE("null") {}
    SUBCASE("non_null") {
      pointer = data.data();
    }
    auto view = BufferView(pointer, 0);
    REQUIRE(is_empty(view));
    auto stream = std::ostringstream();
    stream << view;
    REQUIRE(stream.str().empty());
  }
}
