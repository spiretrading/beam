#include <doctest/doctest.h>
#include "Beam/Queues/CallbackQueueWriter.hpp"

using namespace Beam;

TEST_SUITE("CallbackQueueWriter") {
  TEST_CASE("slot") {
    auto received = 0;
    auto callback = CallbackQueueWriter<int>(
      [&] (auto value) {
        received = value;
      });
    callback.Push(123);
    REQUIRE(received == 123);
  }

  TEST_CASE("break_slot") {
    auto received_value = 0;
    auto received_exception = std::exception_ptr();
    auto callback = CallbackQueueWriter<int>(
      [&] (auto value) {
        received_value = value;
      },
      [&] (auto exception) {
        received_exception = exception;
      });
    callback.Push(12);
    REQUIRE(received_value == 12);
    callback.Break(std::runtime_error("Broken"));
    REQUIRE_THROWS_AS(callback.Push({}), std::runtime_error);
  }
}
