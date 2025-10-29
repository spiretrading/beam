#include <stdexcept>
#include <thread>
#include <vector>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/Utilities/Remote.hpp"

using namespace Beam;

TEST_SUITE("Remote") {
  TEST_CASE("default_constructor") {
    auto remote = Remote<int>();
    REQUIRE(!remote.is_available());
  }

  TEST_CASE("constructor_with_initializer") {
    auto remote = Remote<int>([] (auto& value) {
      value = 42;
    });
    REQUIRE(!remote.is_available());
  }

  TEST_CASE("set_initializer") {
    auto remote = Remote<int>();
    remote.set_initializer([] (auto& value) {
      value = 100;
    });
    REQUIRE(!remote.is_available());
    REQUIRE(*remote == 100);
    REQUIRE(remote.is_available());
  }

  TEST_CASE("dereference_initializes_value") {
    auto initialized = false;
    auto remote = Remote<int>([&] (auto& value) {
      initialized = true;
      value = 42;
    });
    REQUIRE(!initialized);
    REQUIRE(!remote.is_available());
    auto result = *remote;
    REQUIRE(initialized);
    REQUIRE(result == 42);
    REQUIRE(remote.is_available());
  }

  TEST_CASE("arrow_operator_initializes_value") {
    auto remote = Remote<std::string>([] (auto& value) {
      value = "hello";
    });
    REQUIRE(!remote.is_available());
    auto length = remote->length();
    REQUIRE(length == 5);
    REQUIRE(remote.is_available());
  }

  TEST_CASE("multiple_dereferences_initialize_once") {
    auto initialization_count = 0;
    auto remote = Remote<int>([&] (auto& value) {
      ++initialization_count;
      value = 99;
    });
    auto result1 = *remote;
    auto result2 = *remote;
    auto result3 = *remote;
    REQUIRE(initialization_count == 1);
    REQUIRE(result1 == 99);
    REQUIRE(result2 == 99);
    REQUIRE(result3 == 99);
  }

  TEST_CASE("exception_during_initialization") {
    auto initialization_count = 0;
    auto remote = Remote<int>([&] (auto& value) {
      ++initialization_count;
      throw std::runtime_error("initialization failed");
    });
    REQUIRE_THROWS_AS(*remote, std::runtime_error);
    REQUIRE(!remote.is_available());
    REQUIRE(initialization_count == 1);
    REQUIRE_THROWS_AS(*remote, std::runtime_error);
    REQUIRE(initialization_count == 2);
  }

  TEST_CASE("exception_safety_allows_retry") {
    auto attempt = 0;
    auto remote = Remote<int>([&] (auto& value) {
      ++attempt;
      if(attempt < 3) {
        throw std::runtime_error("not ready");
      }
      value = 42;
    });
    REQUIRE_THROWS_AS(*remote, std::runtime_error);
    REQUIRE(!remote.is_available());
    REQUIRE(attempt == 1);
    REQUIRE_THROWS_AS(*remote, std::runtime_error);
    REQUIRE(!remote.is_available());
    REQUIRE(attempt == 2);
    auto result = *remote;
    REQUIRE(remote.is_available());
    REQUIRE(result == 42);
    REQUIRE(attempt == 3);
  }

  TEST_CASE("concurrent_access_with_fast_path") {
    auto remote = Remote<int>([] (auto& value) {
      value = 123;
    });
    auto first_access = *remote;
    REQUIRE(first_access == 123);
    REQUIRE(remote.is_available());
    auto threads = std::vector<std::thread>();
    auto results = std::vector<int>(100);
    for(auto i = 0; i < 100; ++i) {
      threads.emplace_back([&, i] {
        results[i] = *remote;
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    for(auto result : results) {
      REQUIRE(result == 123);
    }
  }

  TEST_CASE("is_available_reflects_state") {
    auto remote = Remote<int>([] (auto& value) {
      value = 10;
    });
    REQUIRE(!remote.is_available());
    auto value = *remote;
    REQUIRE(remote.is_available());
  }

  TEST_CASE("complex_type_initialization") {
    struct Complex {
      int m_field1;
      std::string m_field2;
      std::vector<int> m_field3;
    };
    auto remote = Remote<Complex>([] (auto& value) {
      value = Complex();
      value->m_field1 = 42;
      value->m_field2 = "test";
      value->m_field3 = {1, 2, 3};
    });
    REQUIRE(!remote.is_available());
    REQUIRE(remote->m_field1 == 42);
    REQUIRE(remote->m_field2 == "test");
    REQUIRE(remote->m_field3.size() == 3);
    REQUIRE(remote.is_available());
  }

  TEST_CASE("initializer_receives_boost_optional") {
    auto remote = Remote<int>([] (auto& value) {
      REQUIRE(!value);
      value = 50;
      REQUIRE(value);
      REQUIRE(*value == 50);
    });
    auto result = *remote;
    REQUIRE(result == 50);
  }

  TEST_CASE("concurrent_exception_during_initialization") {
    auto attempt_count = std::atomic<int>(0);
    auto remote = Remote<int>([&] (auto& value) {
      ++attempt_count;
      throw std::runtime_error("always fails");
    });
    auto threads = std::vector<std::thread>();
    auto exception_count = std::atomic<int>(0);
    for(auto i = 0; i < 5; ++i) {
      threads.emplace_back([&] {
        try {
          auto value = *remote;
        } catch(const std::runtime_error&) {
          ++exception_count;
        }
      });
    }
    for(auto& thread : threads) {
      thread.join();
    }
    REQUIRE(exception_count == 5);
    REQUIRE(attempt_count >= 1);
    REQUIRE(!remote.is_available());
  }

  TEST_CASE("arrow_operator_on_pointer_type") {
    auto remote = Remote<std::unique_ptr<int>>([] (auto& value) {
      value = std::make_unique<int>(77);
    });
    REQUIRE(!remote.is_available());
    auto raw_pointer = remote.operator->();
    REQUIRE(remote.is_available());
    REQUIRE(*raw_pointer);
    REQUIRE(**raw_pointer == 77);
  }

  TEST_CASE("move_only_type") {
    auto remote = Remote<std::unique_ptr<int>>([] (auto& value) {
      value = std::make_unique<int>(88);
    });
    REQUIRE(**remote == 88);
    REQUIRE(remote.is_available());
  }

  TEST_CASE("set_initializer_before_access") {
    auto remote = Remote<int>();
    auto first_initialized = false;
    remote.set_initializer([&] (auto& value) {
      first_initialized = true;
      value = 1;
    });
    REQUIRE(*remote == 1);
    REQUIRE(first_initialized);
  }

  TEST_CASE("set_initializer_after_initialization_has_no_effect") {
    auto remote = Remote<int>([] (auto& value) {
      value = 10;
    });
    REQUIRE(*remote == 10);
    auto second_called = false;
    remote.set_initializer([&] (auto& value) {
      second_called = true;
      value = 20;
    });
    REQUIRE(*remote == 10);
    REQUIRE(!second_called);
  }
}
