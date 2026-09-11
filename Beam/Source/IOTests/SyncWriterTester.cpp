#include <doctest/doctest.h>
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SyncWriter.hpp"

using namespace Beam;

TEST_SUITE("SyncWriter") {
  TEST_CASE("write_appends") {
    auto buffer = SharedBuffer();
    auto writer =
      SyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(from<SharedBuffer>("hello"));
    REQUIRE(buffer == "hello");
  }

  TEST_CASE("multiple_writes_append_in_order") {
    auto buffer = SharedBuffer();
    auto writer =
      SyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(from<SharedBuffer>("ab"));
    writer.write(from<SharedBuffer>("cd"));
    writer.write(from<SharedBuffer>("ef"));
    REQUIRE(buffer == "abcdef");
  }

  TEST_CASE("write_zero_bytes_noop") {
    auto buffer = SharedBuffer();
    auto writer =
      SyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    writer.write(SharedBuffer());
    REQUIRE(buffer.get_size() == 0);
  }

  TEST_CASE("write_releases_the_lock") {
    auto buffer = SharedBuffer();
    auto writer =
      SyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
    for(auto i = 0; i != 100; ++i) {
      writer.write(from<SharedBuffer>("a"));
    }
    REQUIRE(buffer.get_size() == 100);
  }
}
