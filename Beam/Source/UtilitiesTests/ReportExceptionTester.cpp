#include <barrier>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Utilities/ReportException.hpp"

using namespace Beam;

namespace {
  struct CustomException : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  class FailingBuffer : public std::streambuf {
    protected:
      std::streamsize xsputn(const char*, std::streamsize) override {
        throw std::runtime_error("write failed");
      }
  };

  class SynchronizedBuffer : public std::stringbuf {
    private:
      mutable std::mutex m_mutex;

      std::streamsize xsputn(const char* data, std::streamsize size) override {
        auto lock = std::lock_guard(m_mutex);
        return std::stringbuf::xsputn(data, size);
      }
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
    auto buffer = SynchronizedBuffer();
    std::cerr.rdbuf(&buffer);
    try {
      function();
      std::cerr.rdbuf(original_buffer);
      return buffer.str();
    } catch(...) {
      std::cerr.rdbuf(original_buffer);
      throw;
    }
  }
}

TEST_SUITE("ReportException") {
  TEST_CASE("log_timestamp") {
    using namespace boost::posix_time;
    auto time = time_from_string("2026-10-15 14:32:08.417999");
    REQUIRE(Details::format_log_timestamp(time) == "2026-10-15T14:32:08.417");
    REQUIRE(Details::format_log_timestamp(
      time_from_string("2026-01-01 00:00:00")) == "2026-01-01T00:00:00.000");
  }

  TEST_CASE("exception_report_output") {
    using namespace boost::posix_time;
    auto exception = std::exception_ptr();
    try {
      try {
        boost::throw_with_location(std::runtime_error("inner failure"));
      } catch(...) {
        std::throw_with_nested(std::runtime_error("outer failure"));
      }
    } catch(...) {
      exception = std::current_exception();
    }
    auto location = std::source_location::current();
    auto before = microsec_clock::local_time();
    auto output = capture_stderr([&] {
      report_exception(exception, "client=test\noperation=read", location);
    });
    auto after = microsec_clock::local_time();
    REQUIRE(output.size() > 23);
    auto timestamp = from_iso_extended_string(output.substr(0, 23));
    REQUIRE(timestamp >= before - milliseconds(1));
    REQUIRE(timestamp <= after);
    auto expected = " Uncaught exception thrown:\n"
      "  client=test\n  operation=read\n" +
      std::format("  reported at: {}:{}\n",
        location.file_name(), location.line()) +
      "  " + Details::indent(make_exception_report(exception), 1);
    if(expected.back() != '\n') {
      expected += '\n';
    }
    REQUIRE(output.substr(23) == expected);
    REQUIRE(output.find("thrown at: ") != std::string::npos);
    REQUIRE(capture_stderr([] {
      report_exception(std::exception_ptr());
    }).empty());
    try {
      std::rethrow_exception(exception);
    } catch(...) {
      auto line = 0;
      output = capture_stderr([&] {
        line = __LINE__ + 1;
        BEAM_LOG_CURRENT_EXCEPTION("request=17");
      });
      REQUIRE(make_exception_report() == make_exception_report(exception));
      REQUIRE(output.find("  request=17\n") != std::string::npos);
      REQUIRE(output.find(std::format("  reported at: {}:{}\n",
        __FILE__, line)) != std::string::npos);
      output = capture_stderr([] {
        BEAM_LOG_CURRENT_EXCEPTION();
      });
      REQUIRE(output.find("reported at: ") != std::string::npos);
    }
  }

  TEST_CASE("concurrent_exception_reports") {
    auto exceptions = std::vector<std::exception_ptr>();
    auto expected = std::vector<std::string>();
    for(auto i = 0; i < 8; ++i) {
      try {
        try {
          throw std::runtime_error(std::format("inner {}", i));
        } catch(...) {
          std::throw_with_nested(
            std::runtime_error(std::format("outer {}", i)));
        }
      } catch(...) {
        exceptions.push_back(std::current_exception());
      }
      auto report = std::format(
        " Uncaught exception thrown:\n  {}\n  ", i) +
        Details::indent(make_exception_report(exceptions.back()), 1);
      if(report.back() != '\n') {
        report += '\n';
      }
      expected.push_back(report);
    }
    auto barrier = std::barrier(8);
    auto output = capture_stderr([&] {
      auto threads = std::vector<std::jthread>();
      for(auto i = std::size_t(0); i < exceptions.size(); ++i) {
        threads.emplace_back([&, i] {
          barrier.arrive_and_wait();
          for(auto j = 0; j < 16; ++j) {
            report_exception(exceptions[i], std::to_string(i),
              std::source_location());
          }
        });
      }
    });
    auto counts = std::vector<int>(exceptions.size());
    auto position = std::size_t(0);
    while(position < output.size()) {
      REQUIRE(output.size() - position >= 23);
      auto match = false;
      for(auto i = std::size_t(0); i < expected.size(); ++i) {
        if(output.compare(
            position + 23, expected[i].size(), expected[i]) == 0) {
          ++counts[i];
          position += 23 + expected[i].size();
          match = true;
          break;
        }
      }
      REQUIRE(match);
    }
    for(auto count : counts) {
      REQUIRE(count == 16);
    }
  }

  TEST_CASE("exception_report_output_failure") {
    auto buffer = FailingBuffer();
    auto original_buffer = std::cerr.rdbuf(&buffer);
    auto original_exceptions = std::cerr.exceptions();
    std::cerr.exceptions(std::ios::badbit | std::ios::failbit);
    auto failure = std::exception_ptr();
    auto report = std::string();
    try {
      throw std::runtime_error("original failure");
    } catch(...) {
      try {
        report_current_exception();
      } catch(...) {
        failure = std::current_exception();
      }
      report = make_exception_report();
    }
    auto failed = std::cerr.bad();
    std::cerr.exceptions(original_exceptions);
    std::cerr.rdbuf(original_buffer);
    REQUIRE(failed);
    REQUIRE(!failure);
    REQUIRE(report.find("original failure") != std::string::npos);
    auto output = capture_stderr([] {
      report_exception(
        std::make_exception_ptr(std::runtime_error("recovered")));
    });
    REQUIRE(output.find("recovered") != std::string::npos);
  }

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
