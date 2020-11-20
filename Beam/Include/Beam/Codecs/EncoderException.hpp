#ifndef BEAM_ENCODER_EXCEPTION_HPP
#define BEAM_ENCODER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Codecs/Codecs.hpp"

namespace Beam::Codecs {

  /** Signals that an error occurred while encoding. */
  class EncoderException : public std::runtime_error, public boost::exception {
    public:
      using runtime_error::runtime_error;

      /** Constructs an EncoderException. */
      EncoderException();
  };

  inline EncoderException::EncoderException()
    : EncoderException("Failed to encode.") {}
}

#endif
