#ifndef BEAM_REPORT_EXCEPTION_HPP
#define BEAM_REPORT_EXCEPTION_HPP
#include <exception>
#include <iostream>
#include <string>
#include <boost/exception/diagnostic_information.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
namespace Details {
  inline std::string Indent(const std::string& value, int level) {
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

  inline std::string MakeExceptionReport(const std::exception_ptr& e,
      int level) {
    auto report = [&] {
      try {
        std::rethrow_exception(e);
      } catch(const boost::exception& e) {
        return std::string(2 * level, ' ') +
          Indent(boost::diagnostic_information(e), level);
      } catch(const std::exception& e) {
        try {
          std::rethrow_if_nested(e);
          return std::string(2 * level, ' ') +
            Indent(boost::diagnostic_information(e), level);
        } catch(const std::exception&) {
          return MakeExceptionReport(std::current_exception(), level);
        }
      }
      return std::string();
    }();
    try {
      std::rethrow_exception(e);
    } catch(const std::exception& e) {
      try {
        std::rethrow_if_nested(e);
        return report;
      } catch(...) {
        return report + MakeExceptionReport(std::current_exception(),
          level + 1);
      }
    } catch(...) {
      return report + MakeExceptionReport(std::current_exception(), level + 1);
    }
  }

  inline void ReportCurrentException(const std::exception& e, int level) {
    std::cerr << std::string(2 * level, ' ') <<
      boost::diagnostic_information(e) << std::endl;
    try {
      std::rethrow_if_nested(e);
    } catch(const std::exception& e) {
      ReportCurrentException(e, level + 1);
    } catch(...) {}
  }
}

  /** Prints to stderr the current exception, including nested exceptions. */
  inline void ReportCurrentException() {
    try {
      if(auto e = std::current_exception()) {
        std::rethrow_exception(e);
      }
    } catch(const std::exception& e) {
      std::cerr << "Uncaught exception thrown:" << std::endl;
      return Details::ReportCurrentException(e, 1);
    } catch(...) {
      try {
        throw std::runtime_error("Unknown exception thrown.");
      } catch(...) {
        ReportCurrentException();
      }
    }
  }

  /**
   * Returns a string containing an exception report,
   * including nested exceptions.
   */
  inline std::string MakeExceptionReport(const std::exception_ptr& e) {
    return Details::MakeExceptionReport(e, 0);
  }

  /**
   * Returns a string containing an exception report for the current exception,
   * including nested exceptions.
   */
  inline std::string MakeExceptionReport() {
    return MakeExceptionReport(std::current_exception());
  }
}

#define BEAM_REPORT_CURRENT_EXCEPTION()                                        \
  __FILE__ << ":" << __LINE__ << " - " << Beam::MakeExceptionReport();

#endif
