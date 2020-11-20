#ifndef BEAM_SERVLET_TEMPLATE_SERVICES_HPP
#define BEAM_SERVLET_TEMPLATE_SERVICES_HPP
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"

namespace Beam {
  inline const auto SERVICE_NAME = std::string("TEMPLATE_SERVICE");

  BEAM_DEFINE_SERVICES(ServletTemplateServices,

    /**
     * Submits a request to echo a message at a specified rate.
     * @param message The message to echo.
     * @param rate The number of times per second to repeat the message.
     * @return Unusued.
     */
    (EchoService, "Beam.ServletTemplate.EchoService", int, std::string, message,
      int, rate));

  BEAM_DEFINE_MESSAGES(ServletTemplateMessages,

    /**
     * Sends an echo'd message.
     * @param message The message that was requested to be echo'd.
     */
    (EchoMessage, "Beam.ServletTemplate.EchoMessage", boost::posix_time::ptime,
      timestamp, std::string, message));
}

#endif
