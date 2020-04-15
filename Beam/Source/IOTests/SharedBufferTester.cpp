#include <cstring>
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::IO;

TEST_SUITE("SharedBuffer") {
  TEST_CASE("create_empty") {
    auto buffer = SharedBuffer();
    REQUIRE(buffer.GetSize() == 0);
    REQUIRE(buffer.GetData() == nullptr);
  }

  TEST_CASE("create_initial_size") {
    auto buffer = SharedBuffer(1);
    REQUIRE(buffer.GetSize() == 1);
    REQUIRE(buffer.GetData() != nullptr);
  }

  TEST_CASE("growing_empty_buffer") {
    auto buffer = SharedBuffer();
    auto size = 0;
    buffer.Grow(1);
    size += 1;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(2);
    size += 2;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(15);
    size += 15;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(1);
    size += 1;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(3141);
    size += 3141;
    REQUIRE(buffer.GetSize() == size);
  }

  TEST_CASE("growing_initial_sized_buffer") {
    auto buffer = SharedBuffer(1);
    auto size = 1;
    buffer.Grow(1);
    size += 1;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(2);
    size += 2;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(15);
    size += 15;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(1);
    size += 1;
    REQUIRE(buffer.GetSize() == size);
    buffer.Grow(3141);
    size += 3141;
    REQUIRE(buffer.GetSize() == size);
  }

  TEST_CASE("copy") {
    auto message = "hello world";
    auto messageSize = static_cast<int>(std::strlen(message));
    auto buffer = SharedBuffer(messageSize);
    std::strncpy(buffer.GetMutableData(), "hello world", messageSize);
    auto copy = buffer;
    REQUIRE(buffer.GetSize() == copy.GetSize());

    // Test copy on write semantics.
    REQUIRE(buffer.GetData() == copy.GetData());
    copy.Append("a", 1);
    REQUIRE(buffer.GetData() != copy.GetData());
  }

  TEST_CASE("append") {
    auto buffer = SharedBuffer();
    auto message = "hello world";
    auto leftMessage = "hello";
    auto rightMessage = " world";
    auto leftMessageSize = static_cast<int>(std::strlen(leftMessage));
    auto rightMessageSize = static_cast<int>(std::strlen(rightMessage));
    buffer.Append(leftMessage, leftMessageSize);
    REQUIRE(buffer.GetSize() == leftMessageSize);
    REQUIRE(std::strncmp(buffer.GetData(), leftMessage, leftMessageSize) == 0);
    buffer.Append(rightMessage, rightMessageSize);
    REQUIRE(buffer.GetSize() == leftMessageSize + rightMessageSize);
    REQUIRE(std::strncmp(buffer.GetData(), message,
      leftMessageSize + rightMessageSize) == 0);
  }

  TEST_CASE("reset") {
    auto buffer = SharedBuffer();
    buffer.Append("a", 1);
    buffer.Reset();
    REQUIRE(buffer.GetSize() == 0);
  }

  TEST_CASE("copy_on_write_with_append_to_original") {
    auto buffer = SharedBuffer();
    buffer.Append("a", 1);
    auto copy = buffer;
    buffer.Append("b", 1);
    REQUIRE(buffer.GetData() != copy.GetData());
  }

  TEST_CASE("copy_on_write_with_append_to_copy") {
    auto buffer = SharedBuffer();
    buffer.Append("a", 1);
    auto copy = buffer;
    copy.Append("b", 1);
    REQUIRE(buffer.GetData() != copy.GetData());
  }
}
