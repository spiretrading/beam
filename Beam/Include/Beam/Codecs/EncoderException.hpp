#ifndef BEAM_ENCODER_EXCEPTION_HPP
#define BEAM_ENCODER_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Signals that an error occurred while encoding. */
  class EncoderException : public std::runtime_error {
    public:
      using runtime_error::runtime_error;

      /** Constructs an EncoderException. */
      EncoderException();
  };

  inline EncoderException::EncoderException()
    : EncoderException("Failed to encode.") {}
}

#endif
