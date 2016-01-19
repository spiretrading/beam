#ifndef BEAM_DATETIME_HPP
#define BEAM_DATETIME_HPP
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  //! Returns the offset between UTC and local time.
  inline boost::posix_time::time_duration GetUtcOffset() {
    boost::posix_time::ptime utcNow =
      boost::posix_time::second_clock::universal_time();
    boost::posix_time::ptime localNow = boost::date_time::c_local_adjustor<
      boost::posix_time::ptime>::utc_to_local(utcNow);
    return localNow - utcNow;
  }
}

#endif
