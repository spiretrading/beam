#ifndef BEAM_REPORT_SERVICE_EXCEPTION_HPP
#define BEAM_REPORT_SERVICE_EXCEPTION_HPP
#include <optional>
#include <sstream>
#include <typeindex>
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {

  /**
   * Reports the current exception with client and service context.
   * @param client The client involved in the failure.
   * @param operation The operation that failed.
   * @param message The message type, if known.
   * @param request The request identifier, if known.
   * @param location The reporting location, defaulting to the caller.
   */
  void report_service_exception(const auto& client, std::string_view operation,
      std::optional<std::type_index> message, std::optional<int> request,
      const std::source_location& location = std::source_location::current()) {
    auto exception = std::current_exception();
    try {
      auto context = std::ostringstream();
      context << "client=" << client.get_identifier() <<
        " operation=" << operation;
      if(message) {
        context << " message=" <<
          client.get_slots().get_registry().get_entry(*message).get_name();
      }
      if(request) {
        context << " request=" << *request;
      }
      report_exception(exception, context.str(), location);
    } catch(...) {
      report_exception(exception, operation, location);
    }
  }

  /** Reports the current exception with client and operation context. */
  void report_service_exception(const auto& client, std::string_view operation,
      const std::source_location& location = std::source_location::current()) {
    report_service_exception(
      client, operation, std::nullopt, std::nullopt, location);
  }
}

#endif
