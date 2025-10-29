#include <sstream>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Beam/Utilities/ReportException.hpp"

using namespace Beam;

namespace {
  struct CustomException : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  void throw_nested_exception() {
    try {
      throw std::runtime_error("inner exception");
    } catch(...) {
      std::throw_with_nested(std::runtime_error("outer exception"));
    }
  }

  void throw_deeply_nested_exception() {
    try {
      try {
        throw std::runtime_error("level 3");
      } catch(...) {
        std::throw_with_nested(std::runtime_error("level 2"));
      }
    } catch(...) {
      std::throw_with_nested(std::runtime_error("level 1"));
    }
  }

  template<typename F>
  std::string capture_stderr(F function) {
    auto original_buffer = std::cerr.rdbuf();
    auto stream = std::stringstream();
    std::cerr.rdbuf(stream.rdbuf());
    try {
      function();
      std::cerr.rdbuf(original_buffer);
      return stream.str();
    } catch(...) {
      std::cerr.rdbuf(original_buffer);
      throw;
    }
  }
}

TEST_SUITE("ReportException") {
  TEST_CASE("make_exception_report_with_exception_ptr") {
    auto exception = std::exception_ptr();
    try {
      throw std::runtime_error("test error");
    } catch(...) {
      exception = std::current_exception();
    }
    auto report = make_exception_report(exception);
    REQUIRE(!report.empty());
    REQUIRE(report.find("test error") != std::string::npos);
  }

  TEST_CASE("make_exception_report_with_current_exception") {
    auto report = std::string();
    try {
      throw std::runtime_error("current exception test");
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("current exception test") != std::string::npos);
  }

  TEST_CASE("make_exception_report_with_nested_exception") {
    auto report = std::string();
    try {
      throw_nested_exception();
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("outer exception") != std::string::npos);
    REQUIRE(report.find("inner exception") != std::string::npos);
  }

  TEST_CASE("make_exception_report_with_deeply_nested_exception") {
    auto report = std::string();
    try {
      throw_deeply_nested_exception();
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("level 1") != std::string::npos);
    REQUIRE(report.find("level 2") != std::string::npos);
    REQUIRE(report.find("level 3") != std::string::npos);
  }

  TEST_CASE("make_exception_report_with_custom_exception") {
    auto report = std::string();
    try {
      throw CustomException("custom error message");
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("custom error message") != std::string::npos);
  }

  TEST_CASE("make_exception_report_preserves_exception_type") {
    auto report = std::string();
    try {
      throw std::logic_error("logic error message");
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("logic error message") != std::string::npos);
  }

  TEST_CASE("report_current_exception_with_simple_exception") {
    auto output = std::string();
    try {
      throw std::runtime_error("simple exception");
    } catch(...) {
      output = capture_stderr([] {
        report_current_exception();
      });
    }
    REQUIRE(!output.empty());
    REQUIRE(output.find("Uncaught exception thrown:") != std::string::npos);
    REQUIRE(output.find("simple exception") != std::string::npos);
  }

  TEST_CASE("report_current_exception_with_nested_exception") {
    auto output = std::string();
    try {
      throw_nested_exception();
    } catch(...) {
      output = capture_stderr([] {
        report_current_exception();
      });
    }
    REQUIRE(!output.empty());
    REQUIRE(output.find("outer exception") != std::string::npos);
    REQUIRE(output.find("inner exception") != std::string::npos);
  }

  TEST_CASE("report_current_exception_without_exception") {
    auto output = capture_stderr([] {
      report_current_exception();
    });
    REQUIRE(output.empty());
  }

  TEST_CASE("report_current_exception_with_unknown_exception") {
    auto output = std::string();
    try {
      throw 42;
    } catch(...) {
      output = capture_stderr([] {
        report_current_exception();
      });
    }
    REQUIRE(!output.empty());
    REQUIRE(output.find("Unknown exception thrown") != std::string::npos);
  }

  TEST_CASE("nested_exception_indentation") {
    auto report = std::string();
    try {
      throw_nested_exception();
    } catch(...) {
      report = make_exception_report();
    }
    auto outer_position = report.find("outer exception");
    auto inner_position = report.find("inner exception");
    REQUIRE(outer_position != std::string::npos);
    REQUIRE(inner_position != std::string::npos);
  }

  TEST_CASE("beam_report_current_exception_macro") {
    auto report = std::string();
    try {
      throw std::runtime_error("macro test");
    } catch(...) {
      auto stream = std::stringstream();
      stream << BEAM_REPORT_CURRENT_EXCEPTION();
      report = stream.str();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("ReportExceptionTester.cpp") != std::string::npos);
    REQUIRE(report.find("macro test") != std::string::npos);
  }

  TEST_CASE("beam_report_current_exception_macro_includes_line_number") {
    auto report = std::string();
    try {
      throw std::runtime_error("line number test");
    } catch(...) {
      auto stream = std::stringstream();
      stream << BEAM_REPORT_CURRENT_EXCEPTION();
      report = stream.str();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find(":") != std::string::npos);
  }

  TEST_CASE("make_exception_report_multiple_times_same_exception") {
    auto exception = std::exception_ptr();
    try {
      throw std::runtime_error("consistent report");
    } catch(...) {
      exception = std::current_exception();
    }
    auto report1 = make_exception_report(exception);
    auto report2 = make_exception_report(exception);
    REQUIRE(report1 == report2);
  }

  TEST_CASE("exception_with_special_characters") {
    auto report = std::string();
    try {
      throw std::runtime_error("error with\nnewline and\ttab");
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("newline") != std::string::npos);
    REQUIRE(report.find("tab") != std::string::npos);
  }

  TEST_CASE("exception_with_empty_message") {
    auto report = std::string();
    try {
      throw std::runtime_error("");
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
  }

  TEST_CASE("report_current_exception_captures_exception_type") {
    auto output = std::string();
    try {
      throw std::invalid_argument("invalid argument test");
    } catch(...) {
      output = capture_stderr([] {
        report_current_exception();
      });
    }
    REQUIRE(!output.empty());
    REQUIRE(output.find("invalid argument test") != std::string::npos);
  }

  TEST_CASE("nested_exceptions_with_different_types") {
    auto report = std::string();
    try {
      try {
        throw std::logic_error("inner logic error");
      } catch(...) {
        std::throw_with_nested(std::runtime_error("outer runtime error"));
      }
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find("outer runtime error") != std::string::npos);
    REQUIRE(report.find("inner logic error") != std::string::npos);
  }

  TEST_CASE("make_exception_report_thread_safety") {
    auto exception1 = std::exception_ptr();
    auto exception2 = std::exception_ptr();
    try {
      throw std::runtime_error("exception 1");
    } catch(...) {
      exception1 = std::current_exception();
    }
    try {
      throw std::runtime_error("exception 2");
    } catch(...) {
      exception2 = std::current_exception();
    }
    auto report1 = make_exception_report(exception1);
    auto report2 = make_exception_report(exception2);
    REQUIRE(report1.find("exception 1") != std::string::npos);
    REQUIRE(report2.find("exception 2") != std::string::npos);
    REQUIRE(report1.find("exception 2") == std::string::npos);
    REQUIRE(report2.find("exception 1") == std::string::npos);
  }

  TEST_CASE("exception_report_long_message") {
    auto long_message = std::string(1000, 'x');
    auto report = std::string();
    try {
      throw std::runtime_error(long_message);
    } catch(...) {
      report = make_exception_report();
    }
    REQUIRE(!report.empty());
    REQUIRE(report.find(long_message) != std::string::npos);
  }

  TEST_CASE("report_current_exception_multiple_levels") {
    auto output = std::string();
    try {
      throw_deeply_nested_exception();
    } catch(...) {
      output = capture_stderr([] {
        report_current_exception();
      });
    }
    REQUIRE(!output.empty());
    REQUIRE(output.find("level 1") != std::string::npos);
    REQUIRE(output.find("level 2") != std::string::npos);
    REQUIRE(output.find("level 3") != std::string::npos);
  }
}
