#include <exception>
#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Services/ServiceRequestException.hpp"

using namespace Beam;

TEST_SUITE("ServiceRequestException") {
  TEST_CASE("default_constructor_has_empty_message") {
    auto exception = ServiceRequestException();
    REQUIRE(std::string(exception.what()) == "");
  }

  TEST_CASE("message_constructor_sets_message") {
    auto exception = ServiceRequestException("failure");
    REQUIRE(std::string(exception.what()) == "failure");
  }

  TEST_CASE("raise_throws_self") {
    auto exception = ServiceRequestException("raised");
    try {
      exception.raise();
      REQUIRE(false);
    } catch(const ServiceRequestException& e) {
      REQUIRE(std::string(e.what()) == "raised");
    }
  }

  TEST_CASE("rethrow_nested_service_exception_wraps_io_exception") {
    try {
      try {
        throw IOException("io-fail");
      } catch(...) {
        rethrow_nested_service_exception("outer-io");
      }
      REQUIRE(false);
    } catch(const std::exception& outer) {
      REQUIRE(std::string(outer.what()) == "outer-io");
      try {
        std::rethrow_if_nested(outer);
        REQUIRE(false);
      } catch(const IOException& inner) {
        REQUIRE(std::string(inner.what()) == "io-fail");
      }
    }
  }

  TEST_CASE(
      "rethrow_nested_service_exception_wraps_service_request_exception") {
    try {
      try {
        throw ServiceRequestException("orig-service");
      } catch(...) {
        rethrow_nested_service_exception("outer-service");
      }
      REQUIRE(false);
    } catch(const ServiceRequestException& outer) {
      REQUIRE(std::string(outer.what()) == "outer-service");
      try {
        std::rethrow_if_nested(outer);
        REQUIRE(false);
      } catch(const ServiceRequestException& inner) {
        REQUIRE(std::string(inner.what()) == "orig-service");
      }
    }
  }

  TEST_CASE("rethrow_nested_service_exception_wraps_std_exception") {
    try {
      try {
        throw std::runtime_error("orig-std");
      } catch(...) {
        rethrow_nested_service_exception("outer-std");
      }
      REQUIRE(false);
    } catch(const std::runtime_error& outer) {
      REQUIRE(std::string(outer.what()) == "outer-std");
      try {
        std::rethrow_if_nested(outer);
        REQUIRE(false);
      } catch(const std::runtime_error& inner) {
        REQUIRE(std::string(inner.what()) == "orig-std");
      }
    }
  }

  TEST_CASE("make_nested_service_exception_returns_exception_ptr") {
    try {
      try {
        throw IOException("io-orig");
      } catch(...) {
        auto eptr = make_nested_service_exception("made-outer");
        REQUIRE(eptr);
        try {
          std::rethrow_exception(eptr);
          REQUIRE(false);
        } catch(const std::exception& outer) {
          REQUIRE(std::string(outer.what()) == "made-outer");
          try {
            std::rethrow_if_nested(outer);
            REQUIRE(false);
          } catch(const IOException& inner) {
            REQUIRE(std::string(inner.what()) == "io-orig");
          }
        }
      }
    } catch(...) {
      REQUIRE(false);
    }
  }

  TEST_CASE("service_or_throw_with_nested_returns_value_on_success") {
    auto result = service_or_throw_with_nested([] { return 123; }, "unused");
    REQUIRE(result == 123);
  }

  TEST_CASE("service_or_throw_with_nested_throws_nested_on_failure") {
    try {
      service_or_throw_with_nested([] {
        throw std::runtime_error("op-fail");
      }, "svc-fail");
      REQUIRE(false);
    } catch(const std::exception& outer) {
      REQUIRE(std::string(outer.what()) == "svc-fail");
      try {
        std::rethrow_if_nested(outer);
        REQUIRE(false);
      } catch(const std::runtime_error& inner) {
        REQUIRE(std::string(inner.what()) == "op-fail");
      }
    }
  }
}
