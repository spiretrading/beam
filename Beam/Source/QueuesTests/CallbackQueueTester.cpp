#include <doctest/doctest.h>
#include "Beam/Queues/CallbackQueue.hpp"

using namespace Beam;

namespace {
  struct DummyException : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
}

TEST_SUITE("CallbackQueue") {
  TEST_CASE("slot") {
    auto callback = CallbackQueue();
    auto received = 0;
    auto slot = callback.GetSlot<int>(
      [&] (auto value) {
        received = value;
      });
    slot.Push(123);
    REQUIRE(received == 123);
  }

  TEST_CASE("break_slot") {
    auto callback = CallbackQueue();
    auto received_value = 0;
    auto received_exception = std::exception_ptr();
    auto slot1 = callback.GetSlot<int>(
      [&] (auto value) {
        received_value = value;
      },
      [&] (auto exception) {
        received_exception = exception;
      });
    slot1.Break(DummyException("Broken"));
    REQUIRE(received_exception != nullptr);
    REQUIRE_THROWS_AS(std::rethrow_exception(received_exception),
      DummyException);
    auto slot2 = callback.GetSlot<int>(
      [&] (auto value) {
        received_value = value + 1;
      },
      [&] (auto exception) {
        received_exception = exception;
      });
    slot2.Push(12);
    REQUIRE(received_value == 13);
    callback.Break(DummyException("Broken"));
    REQUIRE_THROWS_AS(callback.Push({}), DummyException);
    REQUIRE_THROWS_AS(slot2.Push(1), DummyException);
  }
}
