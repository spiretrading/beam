#ifndef BEAM_REPORT_MESSAGE_HPP
#define BEAM_REPORT_MESSAGE_HPP
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <string_view>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Beam {
namespace Details {
  inline std::string format_log_timestamp(boost::posix_time::ptime time) {
    return std::format(
      "{}.{:03}", boost::posix_time::to_iso_extended_string(time).substr(0, 19),
      time.time_of_day().total_milliseconds() % 1000);
  }
}

  /**
   * Prints a message to stderr with a local timestamp.
   * Reporting failures are suppressed.
   * @param message The message to report.
   */
  inline void report_message(std::string_view message) {
    try {
      auto time = boost::posix_time::microsec_clock::local_time();
      if(message.ends_with('\n')) {
        message.remove_suffix(1);
      }
      std::println(
        std::cerr, "{} {}", Details::format_log_timestamp(time), message);
    } catch(...) {}
  }
}

#endif
