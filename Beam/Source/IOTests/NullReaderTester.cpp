#include <doctest/doctest.h>
#include "Beam/IO/NullReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("NullReader") {
  TEST_CASE("read") {
    auto reader = NullReader();
    auto dest = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(dest)), EndOfFileException);
    REQUIRE(dest.get_size() == 0);
  }
}
