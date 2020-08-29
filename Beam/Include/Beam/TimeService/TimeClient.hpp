#ifndef BEAM_TIMECLIENT_HPP
#define BEAM_TIMECLIENT_HPP
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam {
namespace TimeService {

  /*! \struct TimeClient
      \brief Concept for retrieving the current time from a time source.
   */
  struct TimeClient : Concept<TimeClient> {

      //! Returns the current time in UTC.
    boost::posix_time::ptime GetTime();

    void Close();
  };

  //! Truncates a time point.
  /*!
    \param time The time point to truncate.
    \param unit The unit of time to truncate the time point to.
    \return The time point truncated to the specified <i>unit</i>.
  */
  inline boost::posix_time::ptime Truncate(const boost::posix_time::ptime& time,
      boost::posix_time::time_duration unit) {
    return time - boost::posix_time::microseconds(
      time.time_of_day().total_microseconds() % unit.total_microseconds());
  }
}
}

#endif
