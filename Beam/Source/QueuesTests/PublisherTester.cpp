#include <string>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Beam/Queues/Publisher.hpp"

using namespace Beam;

namespace {
  struct TestPublisher : BasePublisher {
    int m_value;

    TestPublisher(int value)
      : m_value(value) {}

    void with(const std::function<void ()>& f) const override {
      f();
    }

    using BasePublisher::with;
  };
}

TEST_SUITE("Publisher") {
  TEST_CASE("void_return") {
    auto publisher = TestPublisher(0);
    auto called = false;
    publisher.with([&] {
      called = true;
    });
    REQUIRE(called);
  }

  TEST_CASE("value_return") {
    auto publisher = TestPublisher(0);
    auto value = publisher.with([&] {
      return 123;
    });
    REQUIRE(value == 123);
  }

  TEST_CASE("string_return") {
    auto publisher = TestPublisher(0);
    auto text = std::string("hello");
    auto result = publisher.with([&] {
      return text + std::string("_world");
    });
    REQUIRE(result == "hello_world");
  }

  TEST_CASE("reference_return") {
    auto publisher = TestPublisher(5);
    auto& reference = publisher.with([&] () -> int& {
      return publisher.m_value;
    });
    REQUIRE(reference == 5);
    reference = 17;
    REQUIRE(publisher.m_value == 17);
  }

  TEST_CASE("const_reference_return") {
    auto publisher = TestPublisher(9);
    auto& reference = publisher.with([&]() -> const int& {
      return publisher.m_value;
    });
    REQUIRE(reference == 9);
  }

  TEST_CASE("exception") {
    auto publisher = TestPublisher(0);
    REQUIRE_THROWS_AS(publisher.with([&]() -> int {
      throw std::runtime_error("failure");
      return 0;
    }), std::runtime_error);
  }
}
