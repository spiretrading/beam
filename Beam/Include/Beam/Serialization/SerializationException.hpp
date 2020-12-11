#ifndef BEAM_SERIALIZATION_EXCEPTION_HPP
#define BEAM_SERIALIZATION_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Serialization/Serialization.hpp"

namespace Beam::Serialization {

  /** Indicates that a serialization operation failed. */
  class SerializationException : public std::runtime_error,
      public boost::exception {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
