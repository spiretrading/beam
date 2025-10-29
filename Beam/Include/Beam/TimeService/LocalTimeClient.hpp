#ifndef BEAM_LOCAL_TIME_CLIENT_HPP
#define BEAM_LOCAL_TIME_CLIENT_HPP
#include "Beam/TimeService/TimeClient.hpp"

namespace Beam {

  /** A TimeClient that uses the local computer's clock. */
  class LocalTimeClient {
    public:

      /** Constructs a LocalTimeClient. */
      LocalTimeClient() = default;

      boost::posix_time::ptime get_time();
      void close();
  };

  inline boost::posix_time::ptime LocalTimeClient::get_time() {
    return boost::posix_time::microsec_clock::universal_time();
  }

  inline void LocalTimeClient::close() {}
}

#endif
