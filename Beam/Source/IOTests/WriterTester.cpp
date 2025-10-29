#include <doctest/doctest.h>
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("Writer") {
  TEST_CASE("write") {
    auto buffer = SharedBuffer();
    auto writer =
      Writer(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    auto data = from<SharedBuffer>("hello");
    writer.write(data);
    REQUIRE(buffer == data);
  }
}
