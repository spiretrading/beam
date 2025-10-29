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
    auto slot = callback.get_slot<int>([&] (auto value) {
      received = value;
    });
    slot.push(123);
    REQUIRE(received == 123);
  }

  TEST_CASE("break_slot") {
    auto callback = CallbackQueue();
    auto received_value = 0;
    auto received_exception = std::exception_ptr();
    auto slot1 = callback.get_slot<int>([&] (auto value) {
      received_value = value;
    }, [&] (auto exception) {
      received_exception = exception;
    });
    slot1.close(DummyException("Broken"));
    REQUIRE(received_exception);
    REQUIRE_THROWS_AS(
      std::rethrow_exception(received_exception), DummyException);
    auto slot2 = callback.get_slot<int>([&] (auto value) {
      received_value = value + 1;
    }, [&] (auto exception) {
      received_exception = exception;
    });
    slot2.push(12);
    REQUIRE(received_value == 13);
    callback.close(DummyException("Broken"));
    REQUIRE_THROWS_AS(callback.push({}), DummyException);
    REQUIRE_THROWS_AS(slot2.push(1), DummyException);
  }
}
