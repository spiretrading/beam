#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Utilities/Active.hpp"

using namespace Beam;

namespace {
  struct Counter {
    std::atomic<int> m_value;

    Counter()
      : m_value(0) {}

    Counter(int value)
      : m_value(value) {}

    Counter(const Counter& other)
      : m_value(other.m_value.load()) {}

    Counter& operator =(const Counter& other) {
      m_value.store(other.m_value.load());
      return *this;
    }

    int get() const {
      return m_value.load();
    }
  };

  struct NonCopyable {
    int m_value;

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

TEST_SUITE("Active") {
  TEST_CASE("default_construct_int") {
    auto active = Active(0);
    auto value = active.load();
    REQUIRE(*value == 0);
  }

  TEST_CASE("construct_with_value") {
    auto active = Active(42);
    auto value = active.load();
    REQUIRE(*value == 42);
  }

  TEST_CASE("construct_string") {
    auto active = Active(std::string("hello"));
    auto value = active.load();
    REQUIRE(*value == "hello");
  }

  TEST_CASE("construct_with_multiple_args") {
    auto active = Active<std::string>(5, 'x');
    auto value = active.load();
    REQUIRE(*value == "xxxxx");
  }

  TEST_CASE("update_value") {
    auto active = Active(10);
    active.update(20);
    auto value = active.load();
    REQUIRE(*value == 20);
  }

  TEST_CASE("update_multiple_times") {
    auto active = Active(1);
    active.update(2);
    active.update(3);
    active.update(4);
    auto value = active.load();
    REQUIRE(*value == 4);
  }

  TEST_CASE("update_string") {
    auto active = Active(std::string("initial"));
    active.update("updated");
    auto value = active.load();
    REQUIRE(*value == "updated");
  }

  TEST_CASE("load_returns_const_shared_ptr") {
    auto active = Active(100);
    auto value = active.load();
    REQUIRE(*value == 100);
    auto same_value = active.load();
    REQUIRE(*same_value == 100);
  }

  TEST_CASE("load_mutable_returns_shared_ptr") {
    auto active = Active(50);
    auto value = active.load();
    REQUIRE(*value == 50);
    *value = 60;
    auto updated = active.load();
    REQUIRE(*updated == 60);
  }

  TEST_CASE("multiple_load_calls_share_same_object") {
    auto active = Active<Counter>(100);
    auto value1 = active.load();
    auto value2 = active.load();
    REQUIRE(value1.get() == value2.get());
    REQUIRE(value1->get() == 100);
    REQUIRE(value2->get() == 100);
  }

  TEST_CASE("update_creates_new_object") {
    auto active = Active<Counter>(10);
    auto value1 = active.load();
    REQUIRE(value1->get() == 10);
    active.update(20);
    auto value2 = active.load();
    REQUIRE(value2->get() == 20);
    REQUIRE(value1->get() == 10);
  }

  TEST_CASE("concurrent_reads") {
    auto active = Active(0);
    auto readers = std::vector<std::thread>();
    auto success = std::atomic_int(0);
    for(auto i = 0; i < 10; ++i) {
      readers.emplace_back([&] {
        for(auto j = 0; j < 100; ++j) {
          auto value = active.load();
          if(*value >= 0) {
            ++success;
          }
        }
      });
    }
    for(auto& thread : readers) {
      thread.join();
    }
    REQUIRE(success == 1000);
  }

  TEST_CASE("concurrent_updates") {
    auto active = Active(0);
    auto writers = std::vector<std::thread>();
    for(auto i = 0; i < 10; ++i) {
      writers.emplace_back([&, i] {
        for(auto j = 0; j < 100; ++j) {
          active.update(i * 100 + j);
        }
      });
    }
    for(auto& thread : writers) {
      thread.join();
    }
    auto final_value = active.load();
    REQUIRE(*final_value >= 0);
    REQUIRE(*final_value < 1000);
  }

  TEST_CASE("concurrent_reads_and_updates") {
    auto active = Active(0);
    auto threads = std::vector<std::thread>();
    auto read_count = std::atomic_int(0);
    for(auto i = 0; i < 5; ++i) {
      threads.emplace_back([&] {
        for(auto j = 0; j < 100; ++j) {
          auto value = active.load();
          if(*value >= 0) {
            ++read_count;
          }
        }
      });
    }
    for(auto i = 0; i < 5; ++i) {
      threads.emplace_back([&, i] {
        for(auto j = 0; j < 100; ++j) {
          active.update(i * 100 + j);
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    REQUIRE(read_count == 500);
    auto final_value = active.load();
    REQUIRE(*final_value >= 0);
  }

  TEST_CASE("update_with_rvalue") {
    auto active = Active(std::string("initial"));
    auto temp = std::string("temporary");
    active.update(std::move(temp));
    auto value = active.load();
    REQUIRE(*value == "temporary");
  }

  TEST_CASE("construct_non_copyable") {
    auto active = Active<NonCopyable>(42);
    auto value = active.load();
    REQUIRE(value->m_value == 42);
  }

  TEST_CASE("update_non_copyable") {
    auto active = Active<NonCopyable>(10);
    active.update(20);
    auto value = active.load();
    REQUIRE(value->m_value == 20);
  }

  TEST_CASE("const_active_load") {
    auto active = Active(123);
    auto const& const_active = active;
    auto value = const_active.load();
    REQUIRE(*value == 123);
  }

  TEST_CASE("old_reference_remains_valid") {
    auto active = Active(1);
    auto old_value = active.load();
    REQUIRE(*old_value == 1);
    active.update(2);
    auto new_value = active.load();
    REQUIRE(*old_value == 1);
    REQUIRE(*new_value == 2);
  }

  TEST_CASE("update_with_constructor_args") {
    auto active = Active(std::string("start"));
    active.update(3, 'a');
    auto value = active.load();
    REQUIRE(*value == "aaa");
  }

  TEST_CASE("stress_test_many_updates") {
    auto active = Active(0);
    for(auto i = 0; i < 10000; ++i) {
      active.update(i);
    }
    auto value = active.load();
    REQUIRE(*value == 9999);
  }

  TEST_CASE("shared_ptr_reference_count") {
    auto active = Active(100);
    {
      auto value1 = active.load();
      auto value2 = active.load();
      REQUIRE(value1.use_count() >= 2);
    }
    active.update(200);
    auto value = active.load();
    REQUIRE(*value == 200);
  }

  TEST_CASE("update_preserves_thread_safety") {
    auto active = Active<Counter>(0);
    auto threads = std::vector<std::thread>();
    for(auto i = 0; i < 10; ++i) {
      threads.emplace_back([&, i] {
        for(auto j = 0; j < 100; ++j) {
          active.update(i);
          auto value = active.load();
          REQUIRE(value->get() >= 0);
          REQUIRE(value->get() < 10);
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
  }

  TEST_CASE("load_during_update") {
    auto active = Active(std::string("initial"));
    auto threads = std::vector<std::thread>();
    auto successful_reads = std::atomic_int(0);
    for(auto i = 0; i < 5; ++i) {
      threads.emplace_back([&] {
        for(auto j = 0; j < 200; ++j) {
          auto value = active.load();
          if(!value->empty()) {
            ++successful_reads;
          }
        }
      });
    }
    for(auto i = 0; i < 5; ++i) {
      threads.emplace_back([&, i] {
        for(auto j = 0; j < 200; ++j) {
          active.update("update_" + std::to_string(i * 200 + j));
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    REQUIRE(successful_reads == 1000);
  }
}
