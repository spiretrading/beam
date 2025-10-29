#include <doctest/doctest.h>
#include "Beam/IO/AsyncWriter.hpp"
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("AsyncWriter") {
  TEST_CASE("write_raw_appends") {
    auto buffer = SharedBuffer();
    {
      auto writer =
        AsyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
      writer.write(from<SharedBuffer>("hello"));
    }
    REQUIRE(buffer == "hello");
  }

  TEST_CASE("write_buffer_appends") {
    auto buffer = SharedBuffer();
    {
      auto writer =
        AsyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
      auto source = from<SharedBuffer>("abc");
      writer.write(source);
    }
    REQUIRE(buffer == "abc");
  }

  TEST_CASE("write_move_appends") {
    auto buffer = SharedBuffer();
    {
      auto writer =
        AsyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
      writer.write(from<SharedBuffer>("xyz"));
    }
    REQUIRE(buffer == "xyz");
  }

  TEST_CASE("multiple_writes_append_in_order") {
    auto buffer = SharedBuffer();
    {
      auto writer =
        AsyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
      writer.write(from<SharedBuffer>("ab"));
      writer.write(from<SharedBuffer>("cd"));
      writer.write(from<SharedBuffer>("ef"));
    }
    REQUIRE(buffer == "abcdef");
  }

  TEST_CASE("write_zero_bytes_noop") {
    auto buffer = SharedBuffer();
    {
      auto writer =
        AsyncWriter(std::make_unique<BufferWriter<SharedBuffer>>(Ref(buffer)));
      writer.write(SharedBuffer());
    }
    REQUIRE(buffer.get_size() == 0);
  }
}
