#ifndef BEAM_SERIALIZATION_EXCEPTION_HPP
#define BEAM_SERIALIZATION_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Indicates that a serialization operation failed. */
  class SerializationException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
