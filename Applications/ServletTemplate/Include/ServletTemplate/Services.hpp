#ifndef BEAM_SERVLETTEMPLATESERVICES_HPP
#define BEAM_SERVLETTEMPLATESERVICES_HPP
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"

namespace Beam {
  static const std::string SERVICE_NAME = "TEMPLATE_SERVICE";

  BEAM_DEFINE_SERVICES(ServletTemplateServices,

    /*! \interface Beam::ServletTemplate::EchoService
        \brief Submits a request to echo a message at a specified rate.
        \param message <code>std::string</code> The message to echo.
        \param rate The number of times per second to repeat the message.
        \return <code>int</code> unusued.
    */
    //! \cond
    (EchoService, "Beam.ServletTemplate.EchoService", int, std::string, message,
      int, rate));
    //! \endcond

  BEAM_DEFINE_MESSAGES(ServletTemplateMessages,

    /*! \interface Beam::ServletTemplate::EchoMessage
        \brief Sends an echo'd message.
        \param message The message that was requested to be echo'd.
    */
    //! \cond
    (EchoMessage, "Beam.ServletTemplate.EchoMessage", boost::posix_time::ptime,
      timestamp, std::string, message));
    //! \endcond
}

#endif
