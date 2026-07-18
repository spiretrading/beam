module;
#include "Prelude.hpp"

export module Beam:NullConnection;

export namespace Beam {

  /** A Connection used for testing purposes. */
  class NullConnection {
    public:
      void close();
  };

  inline void NullConnection::close() {}
}

