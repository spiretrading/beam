#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/BufferOutputStream.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BufferOutputStream") {
  TEST_CASE("write") {
    auto buffer = SharedBuffer();
    auto stream = SharedBufferOutputStream(Ref(buffer));
    stream << "hello" << std::flush;
    REQUIRE(buffer == "hello");
    stream << " world" << std::flush;
    REQUIRE(buffer == "hello world");
  }
}
