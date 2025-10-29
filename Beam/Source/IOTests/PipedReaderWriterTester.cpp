#include <future>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"

using namespace Beam;

TEST_SUITE("PipedReaderWriter") {
  TEST_CASE("write_then_read") {
    auto reader = PipedReader();
    auto writer = PipedWriter(Ref(reader));
    auto writer_task = std::async(std::launch::async, [&] {
      writer.write(from<SharedBuffer>("hello world"));
    });
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    writer_task.get();
    REQUIRE(buffer == "hello world");
  }

  TEST_CASE("multiple_messages") {
    auto reader = PipedReader();
    auto writer = PipedWriter(Ref(reader));
    auto writer_task = std::async(std::launch::async, [&] {
      writer.write(from<SharedBuffer>("one"));
      writer.write(from<SharedBuffer>("two"));
      writer.write(from<SharedBuffer>("three"));
    });
    auto first = SharedBuffer();
    reader.read(out(first));
    auto second = SharedBuffer();
    reader.read(out(second));
    auto third = SharedBuffer();
    reader.read(out(third));
    writer_task.get();
    REQUIRE(first == "one");
    REQUIRE(second == "two");
    REQUIRE(third == "three");
  }

  TEST_CASE("close_with_exception") {
    auto reader = PipedReader();
    auto writer = PipedWriter(Ref(reader));
    auto closer = std::async(std::launch::async, [&] {
      writer.close(std::runtime_error("boom"));
    });
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(buffer)), std::runtime_error);
    closer.get();
  }

  TEST_CASE("poll_after_write") {
    auto reader = PipedReader();
    auto writer = PipedWriter(Ref(reader));
    auto write_task = std::async(std::launch::async, [&] {
      writer.write(from<SharedBuffer>("ping"));
    });
    write_task.wait();
    REQUIRE(reader.poll());
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    REQUIRE(buffer == "ping");
  }
}
