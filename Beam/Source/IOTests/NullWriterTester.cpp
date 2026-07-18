module;
#include "Prelude.hpp"
#include <doctest/doctest.h>
#include <string>
#include "Beam/IO/SharedBuffer.hpp"

module Beam;

using namespace Beam;

TEST_SUITE("NullWriter") {
  TEST_CASE("write") {
    auto writer = NullWriter();
    auto buffer = from<SharedBuffer>("payload");
    REQUIRE_NOTHROW(writer.write(buffer));
  }
}
