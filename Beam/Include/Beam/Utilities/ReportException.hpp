#ifndef BEAM_REPORT_EXCEPTION_HPP
#define BEAM_REPORT_EXCEPTION_HPP
#include <exception>
#include <iostream>
#include <string>
#include <boost/exception/diagnostic_information.hpp>
#include "Beam/Utilities/Utilities.hpp"

#define BEAM_REPORT_CURRENT_EXCEPTION()                                        \
  __FILE__ << ":" << __LINE__ << " - " <<                                      \
  boost::current_exception_diagnostic_information()

namespace Beam {
namespace Details {
  inline void ReportCurrentException(const std::exception& e, int level) {
    std::cerr << std::string(2 * level, ' ') << e.what() << std::endl;
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
}

#endif
