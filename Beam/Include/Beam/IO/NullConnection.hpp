#ifndef BEAM_NULL_CONNECTION_HPP
#define BEAM_NULL_CONNECTION_HPP
#include "Beam/IO/Connection.hpp"

namespace Beam {
namespace IO {

  /** A Connection used for testing purposes. */
  class NullConnection {
    public:
      void Close();
  };

  inline void NullConnection::Close() {}
}

  template<>
  struct ImplementsConcept<IO::NullConnection, IO::Connection> :
    std::true_type {};
}

#endif
