#ifndef BEAM_SERVICEPROTOCOLPROFILERSERVICES_HPP
#define BEAM_SERVICEPROTOCOLPROFILERSERVICES_HPP
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"

namespace Beam {
  BEAM_DEFINE_SERVICES(ServiceProtocolProfilerServices,

    /*! \interface Beam::ServiceProtocolProfiler::EchoService
        \brief Submits a request to echo a message.
        \param message <code>std::string</code> The message to echo.
        \return <code>std::string</code> The message received.
    */
    //! \cond
    (EchoService, "Beam.ServiceProtocolProfiler.EchoService", std::string,
      std::string, message));
    //! \endcond

  BEAM_DEFINE_MESSAGES(ServiceProtocolProfilerMessages,

    /*! \interface Beam::ServiceProtocolProfiler::EchoMessage
        \brief Sends a message.
        \param message The message to send.
    */
    //! \cond
    (EchoMessage, "Beam.ServiceProtocolProfiler.EchoMessage",
      boost::posix_time::ptime, timestamp, std::string, message));
    //! \endcond
}

#endif
