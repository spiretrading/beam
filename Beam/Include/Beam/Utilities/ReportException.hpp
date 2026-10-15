#ifndef BEAM_REPORT_EXCEPTION_HPP
#define BEAM_REPORT_EXCEPTION_HPP
#include <exception>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Utilities/ReportMessage.hpp"

namespace Beam {
namespace Details {
  inline std::string indent(std::string_view value, int level) {
    auto output = std::string();
    auto last = '\0';
    for(auto c : value) {
      if(last == '\n') {
        output += std::string(2 * level, ' ');
      }
      last = c;
      output += c;
    }
    return output;
  }

  inline std::string make_location_report(const std::exception& e) {
    auto location = boost::get_throw_location(e);
    if(location.line() == 0) {
      return std::string();
    }
    return "thrown at: " + std::string(location.file_name()) + ":" +
      std::to_string(location.line()) + "\n";
  }

  inline std::string make_exception_report(
      const std::exception_ptr& e, int level) {
    if(!e) {
      return std::string();
    }
    auto report = [&] {
      try {
        std::rethrow_exception(e);
      } catch(const std::exception& e) {
        return std::string(2 * level, ' ') +
          indent(boost::diagnostic_information(e) + make_location_report(e),
            level);
      } catch(...) {
        return std::string(2 * level, ' ') + "Unknown exception thrown.\n";
      }
    }();
    try {
      std::rethrow_exception(e);
    } catch(const std::exception& e) {
      try {
        std::rethrow_if_nested(e);
      } catch(...) {
        return report + "\n" +
          make_exception_report(std::current_exception(), level + 1);
      }
    } catch(...) {}
    return report;
  }
}

  /**
   * Returns a string containing an exception report, including nested
   * exceptions.
   */
  inline std::string make_exception_report(const std::exception_ptr& e) {
    return Details::make_exception_report(e, 0);
  }

  /**
   * Returns a string containing an exception report for the current exception,
   * including nested exceptions.
   */
  inline std::string make_exception_report() {
    return make_exception_report(std::current_exception());
  }

  /**
   * Prints a timestamped exception report to stderr, including nested causes.
   * Reporting failures are suppressed.
   * @param e The exception to report, or an empty pointer to report nothing.
   * @param context Additional context, or an empty string.
   * @param location The reporting location, or an empty source location.
   */
  inline void report_exception(const std::exception_ptr& e,
      std::string_view context, const std::source_location& location) {
    if(!e) {
      return;
    }
    try {
      auto report = std::string("Uncaught exception thrown:\n");
      if(!context.empty()) {
        report += "  " + Details::indent(context, 1) + "\n";
      }
      if(location.line() != 0) {
        report += std::format("  reported at: {}:{}\n",
          location.file_name(), location.line());
      }
      report += "  " + Details::indent(make_exception_report(e), 1);
      report_message(report);
    } catch(...) {}
  }

  /** Prints a timestamped exception report to stderr. */
  inline void report_exception(const std::exception_ptr& e) {
    report_exception(e, std::string_view(), std::source_location());
  }

  /**
   * Prints the current exception to stderr with a local timestamp and context.
   * @param context Additional context, or an empty string.
   * @param location The reporting location.
   */
  inline void report_current_exception(
      std::string_view context, const std::source_location& location) {
    report_exception(std::current_exception(), context, location);
  }

  /** Prints the current exception to stderr with its reporting location. */
  inline void report_current_exception(const std::source_location& location) {
    report_current_exception(std::string_view(), location);
  }

  /** Prints the current exception to stderr with a local timestamp. */
  inline void report_current_exception() {
    report_exception(std::current_exception());
  }
}

#define BEAM_REPORT_CURRENT_EXCEPTION()                                        \
  __FILE__ << ":" << __LINE__ << " - " << Beam::make_exception_report()

#define BEAM_LOG_CURRENT_EXCEPTION(...)                                        \
  ::Beam::report_current_exception(                                            \
    __VA_ARGS__ __VA_OPT__(,) std::source_location::current())

#endif
