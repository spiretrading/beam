#ifndef BEAM_SERVICE_PROTOCOL_PROFILER_SERVICES_HPP
#define BEAM_SERVICE_PROTOCOL_PROFILER_SERVICES_HPP
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"

namespace Beam {
  BEAM_DEFINE_SERVICES(service_protocol_profiler_services,

    /**
     * Submits a request to echo a message.
     * @param message The message to echo.
     * @return The message received.
     */
    (EchoService, "Beam.ServiceProtocolProfiler.EchoService", std::string,
      (std::string, message)));

  BEAM_DEFINE_MESSAGES(service_protocol_profiler_messages,

    /**
     * Sends a message.
     * @param message The message to send.
     */
    (EchoMessage, "Beam.ServiceProtocolProfiler.EchoMessage",
      (boost::posix_time::ptime, timestamp), (std::string, message)));
}

#endif
