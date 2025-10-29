#include <functional>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Beam/SignalHandling/ScopedSlotAdaptor.hpp"

using namespace Beam;

TEST_SUITE("ScopedSlotAdaptor") {
  TEST_CASE("invoke") {
    auto count = 0;
    auto slot = std::function<int (int)>();
    {
      auto adaptor = ScopedSlotAdaptor();
      auto base_slot = adaptor.make_slot([&] (int value) {
        count += value;
        return value * 2;
      });
      slot = std::function<int (int)>(
        [base_slot = std::move(base_slot)] (int value) mutable {
          return base_slot(value);
        });
      auto result = slot(3);
      REQUIRE(result == 6);
      REQUIRE(count == 3);
    }
    REQUIRE_THROWS_AS(slot(1), std::runtime_error);
  }

  TEST_CASE("temporary_adaptor_produces_expired_slot") {
    auto slot = [&] {
      auto adaptor = ScopedSlotAdaptor();
      return adaptor.make_slot([] {
        return 0;
      });
    }();
    REQUIRE_THROWS_AS(slot(), std::runtime_error);
  }

  TEST_CASE("multiple_slots_share_lifetime_and_expire_together") {
    auto slot_a = std::function<void (int)>();
    auto slot_b = std::function<void (int)>();
    {
      auto count_a = 0;
      auto count_b = 0;
      auto adaptor = ScopedSlotAdaptor();
      auto base_slot_a = adaptor.make_slot([&] (int value) {
        count_a += value;
      });
      auto base_slot_b = adaptor.make_slot([&] (int value) {
        count_b += value;
      });
      slot_a = std::function<void (int)>(
        [base_slot_a = std::move(base_slot_a)] (int value) mutable {
          base_slot_a(value);
        });
      slot_b = std::function<void (int)>(
        [base_slot_b = std::move(base_slot_b)] (int value) mutable {
          base_slot_b(value);
        });
      slot_a(2);
      slot_b(3);
      REQUIRE(count_a == 2);
      REQUIRE(count_b == 3);
    }
    REQUIRE_THROWS_AS(slot_a(1), std::runtime_error);
    REQUIRE_THROWS_AS(slot_b(1), std::runtime_error);
  }
}
