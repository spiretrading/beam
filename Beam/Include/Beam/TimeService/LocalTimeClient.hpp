#ifndef BEAM_LOCAL_TIME_CLIENT_HPP
#define BEAM_LOCAL_TIME_CLIENT_HPP
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "Beam/TimeService/TimeService.hpp"

namespace Beam::TimeService {

  /** A TimeClient that uses the local computer's clock. */
  class LocalTimeClient {
    public:

      /** Constructs a LocalTimeClient. */
      LocalTimeClient() = default;

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();
  };

  inline boost::posix_time::ptime LocalTimeClient::GetTime() {
    return boost::posix_time::microsec_clock::universal_time();
  }

  inline void LocalTimeClient::Open() {}

  inline void LocalTimeClient::Close() {}
}

#endif
