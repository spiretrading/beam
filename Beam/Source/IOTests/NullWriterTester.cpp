#include <string>
#include <doctest/doctest.h>
#include "Beam/IO/NullWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("NullWriter") {
  TEST_CASE("write") {
    auto writer = NullWriter();
    auto buffer = from<SharedBuffer>("payload");
    REQUIRE_NOTHROW(writer.write(buffer));
  }
}
