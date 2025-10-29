#ifndef BEAM_NULL_CONNECTION_HPP
#define BEAM_NULL_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"

namespace Beam {

  /** A Connection used for testing purposes. */
  class NullConnection {
    public:
      void close();
  };

  inline void NullConnection::close() {}
}

#endif
