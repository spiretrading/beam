module;
#include "Prelude.hpp"

export module Beam:LocalTimeClient;

export namespace Beam {

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

