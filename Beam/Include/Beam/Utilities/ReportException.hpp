#ifndef BEAM_REPORT_EXCEPTION_HPP
#define BEAM_REPORT_EXCEPTION_HPP
#include <exception>
#include <iostream>
#include <string_view>
#include <boost/exception/diagnostic_information.hpp>

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

  inline std::string make_exception_report(
      const std::exception_ptr& e, int level) {
    auto report = [&] {
      try {
        std::rethrow_exception(e);
      } catch(const std::exception& e) {
        return std::string(2 * level, ' ') +
          indent(boost::diagnostic_information(e), level);
      }
      return std::string();
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

  inline void report_current_exception(const std::exception& e, int level) {
    std::cerr << std::string(2 * level, ' ') <<
      boost::diagnostic_information(e) << std::endl;
    try {
      std::rethrow_if_nested(e);
    } catch(const std::exception& e) {
      report_current_exception(e, level + 1);
    } catch(...) {}
  }
}

  /** Prints to stderr the current exception, including nested exceptions. */
  inline void report_current_exception() {
    try {
      if(auto e = std::current_exception()) {
        std::rethrow_exception(e);
      }
    } catch(const std::exception& e) {
      std::cerr << "Uncaught exception thrown:" << std::endl;
      return Details::report_current_exception(e, 1);
    } catch(...) {
      try {
        boost::throw_with_location(
          std::runtime_error("Unknown exception thrown."));
      } catch(...) {
        report_current_exception();
      }
    }
  }

  /**
   * Returns a string containing an exception report,
   * including nested exceptions.
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
}

#define BEAM_REPORT_CURRENT_EXCEPTION()                                        \
  __FILE__ << ":" << __LINE__ << " - " << Beam::make_exception_report()

#endif
