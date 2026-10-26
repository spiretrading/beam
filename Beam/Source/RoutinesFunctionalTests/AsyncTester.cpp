#include <doctest/doctest.h>
#include "Beam/Routines/Async.hpp"

using namespace Beam;

namespace {
  struct AddressValue {
    AddressValue* m_address;

    AddressValue() noexcept
        : m_address() {
      m_address = this;
    }
  };

  struct ImmovableValue {
    int m_value;

    explicit ImmovableValue(int value) noexcept
      : m_value(value) {}

    ImmovableValue(const ImmovableValue&) = delete;
  };

  struct Conversion {
    Async<void>* m_ready;
    Async<void>* m_release;

    operator int() const {
      m_ready->get_eval().set();
      m_release->get();
      return 42;
    }
  };

  struct Value {
    int m_value;

    Value(Async<void>& ready, Async<void>& release)
        : m_value(42) {
      ready.get_eval().set();
      release.get();
    }
  };

  struct Copy {
    Async<void>* m_ready;
    Async<void>* m_release;
    int m_value;

    Copy(Async<void>& ready, Async<void>& release)
      : m_ready(&ready),
        m_release(&release),
        m_value(42) {}

    Copy(const Copy& value)
        : m_ready(value.m_ready),
          m_release(value.m_release),
          m_value(value.m_value) {
      m_ready->get_eval().set();
      m_release->get();
    }
  };

  template<typename T>
  int test_suspension(auto publish, auto read) {
    auto async = Async<T>();
    auto ready = Async<void>();
    auto release = Async<void>();
    auto result = Async<int>();
    auto producer = spawn([&] {
      auto eval = async.get_eval();
      publish(eval, ready, release);
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    ready.get();
    auto consumer = spawn([&] {
      return read(async.get());
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0, result.get_eval());
    auto notifier = spawn([&] {
      release.get_eval().set();
    }, Details::Scheduler::DEFAULT_STACK_SIZE, 0);
    auto received = result.get();
    wait(producer);
    wait(consumer);
    wait(notifier);
    return received;
  }
}

TEST_SUITE("Async") {
  TEST_CASE("emplace_preserves_address") {
    auto async = Async<AddressValue>();
    async.get_eval().emplace();
    auto& value = async.get();
    REQUIRE(value.m_address == &value);
  }

  TEST_CASE("emplace_immovable_value") {
    auto async = Async<ImmovableValue>();
    async.get_eval().emplace(42);
    REQUIRE(async.get().m_value == 42);
  }

  TEST_CASE_TEMPLATE("conversion_suspends", T, int, bool) {
    auto emplace = false;
    SUBCASE("set") {}
    SUBCASE("emplace") {
      emplace = true;
    }
    auto result = test_suspension<T>(
      [&] (auto& eval, auto& ready, auto& release) {
        auto value = Conversion(&ready, &release);
        if(emplace) {
          eval.emplace(value);
        } else {
          eval.set(value);
        }
      }, [] (const auto& value) {
        return static_cast<int>(value);
      });
    REQUIRE(result == static_cast<int>(T(42)));
  }

  TEST_CASE("construction_suspends") {
    auto result = test_suspension<Value>(
      [] (auto& eval, auto& ready, auto& release) {
        eval.emplace(ready, release);
      }, [] (const auto& value) {
        return value.m_value;
      });
    REQUIRE(result == 42);
  }

  TEST_CASE("copy_suspends") {
    auto result = test_suspension<Copy>(
      [] (auto& eval, auto& ready, auto& release) {
        auto value = Copy(ready, release);
        eval.set(value);
      }, [] (const auto& value) {
        return value.m_value;
      });
    REQUIRE(result == 42);
  }
}
