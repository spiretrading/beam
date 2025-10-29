#include <atomic>
#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Utilities/CachedValue.hpp"

using namespace Beam;

namespace {
  struct Counter {
    std::atomic<int> m_count;

    Counter()
      : m_count(0) {}

    int operator ()() {
      return ++m_count;
    }
  };

  struct ThrowingConstructor {
    ThrowingConstructor() {
      throw std::runtime_error("construction failed");
    }
  };

  struct NonCopyable {
    int m_value;

    NonCopyable()
      : m_value(0) {}

    NonCopyable(int value)
      : m_value(value) {}

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator =(const NonCopyable&) = delete;

    NonCopyable(NonCopyable&& other) noexcept
      : m_value(other.m_value) {
      other.m_value = 0;
    }

    NonCopyable& operator =(NonCopyable&& other) noexcept {
      m_value = other.m_value;
      other.m_value = 0;
      return *this;
    }
  };
}

TEST_SUITE("CachedValue") {
  TEST_CASE("default_construct") {
    auto cached = CachedValue<int>();
    cached.set_computation([] { return 42; });
    REQUIRE(*cached == 42);
  }

  TEST_CASE("construct_with_computation") {
    auto cached = CachedValue([] { return 100; });
    REQUIRE(*cached == 100);
  }

  TEST_CASE("construct_with_string") {
    auto cached = CachedValue([] { return std::string("test"); });
    REQUIRE(*cached == "test");
  }

  TEST_CASE("dereference_operator") {
    auto cached = CachedValue([] { return 42; });
    REQUIRE(*cached == 42);
  }

  TEST_CASE("arrow_operator") {
    auto cached = CachedValue([] { return std::string("hello"); });
    REQUIRE(cached->size() == 5);
  }

  TEST_CASE("get_method") {
    auto cached = CachedValue([] { return 123; });
    auto value = cached.get();
    REQUIRE(*value == 123);
  }

  TEST_CASE("computation_called_once") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    REQUIRE(*cached == 1);
    REQUIRE(*cached == 1);
    REQUIRE(*cached == 1);
    REQUIRE(counter.m_count == 1);
  }

  TEST_CASE("multiple_accesses_same_value") {
    auto cached = CachedValue([] { return 42; });
    REQUIRE(*cached == 42);
    REQUIRE(cached.get());
    REQUIRE(*cached.get() == 42);
  }

  TEST_CASE("set_computation") {
    auto cached = CachedValue([] { return 10; });
    REQUIRE(*cached == 10);
    cached.set_computation([] { return 20; });
    REQUIRE(*cached == 20);
  }

  TEST_CASE("set_computation_invalidates_cache") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    REQUIRE(*cached == 1);
    REQUIRE(*cached == 1);
    cached.set_computation(std::ref(counter));
    REQUIRE(*cached == 2);
  }

  TEST_CASE("invalidate") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    REQUIRE(*cached == 1);
    REQUIRE(*cached == 1);
    cached.invalidate();
    REQUIRE(*cached == 2);
    REQUIRE(*cached == 2);
  }

  TEST_CASE("invalidate_multiple_times") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    REQUIRE(*cached == 1);
    cached.invalidate();
    REQUIRE(*cached == 2);
    cached.invalidate();
    REQUIRE(*cached == 3);
    cached.invalidate();
    REQUIRE(*cached == 4);
  }

  TEST_CASE("invalidate_without_access") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    cached.invalidate();
    cached.invalidate();
    cached.invalidate();
    REQUIRE(*cached == 1);
    REQUIRE(counter.m_count == 1);
  }

  TEST_CASE("const_cached_value") {
    auto const cached = CachedValue([] { return 42; });
    REQUIRE(*cached == 42);
  }

  TEST_CASE("cache_with_complex_type") {
    auto cached = CachedValue([] { return std::string("complex_value"); });
    REQUIRE(*cached == "complex_value");
    REQUIRE(cached->length() == 13);
  }

  TEST_CASE("cache_computed_result") {
    auto computation_count = 0;
    auto cached = CachedValue([&] {
      ++computation_count;
      return 2 + 2;
    });
    REQUIRE(*cached == 4);
    REQUIRE(*cached == 4);
    REQUIRE(*cached == 4);
    REQUIRE(computation_count == 1);
  }

  TEST_CASE("cache_with_state") {
    auto state = 0;
    auto cached = CachedValue([&] {
      return ++state * 10;
    });
    REQUIRE(*cached == 10);
    REQUIRE(*cached == 10);
    cached.invalidate();
    REQUIRE(*cached == 20);
  }

  TEST_CASE("get_returns_pointer") {
    auto cached = CachedValue([] { return 99; });
    auto pointer = cached.get();
    REQUIRE(pointer);
    REQUIRE(*pointer == 99);
  }

  TEST_CASE("multiple_get_calls") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    auto p1 = cached.get();
    auto p2 = cached.get();
    auto p3 = cached.get();
    REQUIRE(*p1 == 1);
    REQUIRE(*p2 == 1);
    REQUIRE(*p3 == 1);
    REQUIRE(counter.m_count == 1);
  }

  TEST_CASE("arrow_operator_with_struct") {
    struct Data {
      int m_value;
      std::string m_name;
      int get_double() const { return m_value * 2; }
    };
    auto cached = CachedValue<Data>([] {
      return Data(42, "test");
    });
    REQUIRE(cached->m_value == 42);
    REQUIRE(cached->m_name == "test");
    REQUIRE(cached->get_double() == 84);
  }

  TEST_CASE("change_computation_after_use") {
    auto cached = CachedValue([] { return std::string("first"); });
    REQUIRE(*cached == "first");
    cached.set_computation([] { return std::string("second"); });
    REQUIRE(*cached == "second");
    cached.set_computation([] { return std::string("third"); });
    REQUIRE(*cached == "third");
  }

  TEST_CASE("cache_zero_value") {
    auto cached = CachedValue([] { return 0; });
    REQUIRE(*cached == 0);
  }

  TEST_CASE("cache_empty_string") {
    auto cached = CachedValue([] { return std::string(); });
    REQUIRE(cached->empty());
    REQUIRE(cached->size() == 0);
  }

  TEST_CASE("invalidate_then_set_new_computation") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    REQUIRE(*cached == 1);
    cached.invalidate();
    cached.set_computation([] { return 999; });
    REQUIRE(*cached == 999);
  }

  TEST_CASE("computation_with_exception") {
    auto cached = CachedValue([] {
      throw std::runtime_error("computation error");
      return 0;
    });
    REQUIRE_THROWS_AS(*cached, std::runtime_error);
  }

  TEST_CASE("recompute_after_exception") {
    auto should_throw = true;
    auto cached = CachedValue([&] {
      if(should_throw) {
        throw std::runtime_error("error");
      }
      return 42;
    });
    REQUIRE_THROWS_AS(*cached, std::runtime_error);
    should_throw = false;
    cached.invalidate();
    REQUIRE(*cached == 42);
  }

  TEST_CASE("cache_move_only_type") {
    auto cached = CachedValue([] { return NonCopyable(123); });
    REQUIRE(cached->m_value == 123);
  }

  TEST_CASE("different_access_methods_same_result") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    auto& ref = *cached;
    auto* ptr = cached.get();
    auto& ref2 = *cached;
    REQUIRE(ref == 1);
    REQUIRE(*ptr == 1);
    REQUIRE(ref2 == 1);
    REQUIRE(counter.m_count == 1);
  }

  TEST_CASE("cache_with_mutable_lambda") {
    auto cached = CachedValue([counter = 0]() mutable {
      return ++counter;
    });
    REQUIRE(*cached == 1);
    REQUIRE(*cached == 1);
    cached.invalidate();
    REQUIRE(*cached == 2);
  }

  TEST_CASE("default_construct_then_set") {
    auto cached = CachedValue<int>();
    cached.set_computation([] { return 777; });
    REQUIRE(*cached == 777);
  }

  TEST_CASE("invalidate_before_first_access") {
    auto counter = Counter();
    auto cached = CachedValue(std::ref(counter));
    cached.invalidate();
    REQUIRE(*cached == 1);
  }

  TEST_CASE("const_correctness") {
    auto const cached = CachedValue([] { return 42; });
    auto const& ref = *cached;
    auto const* ptr = cached.get();
    REQUIRE(ref == 42);
    REQUIRE(*ptr == 42);
  }
}
