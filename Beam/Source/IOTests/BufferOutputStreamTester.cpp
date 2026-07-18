module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>
#include "Beam/IO/SharedBuffer.hpp"

module Beam;

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
