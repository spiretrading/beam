#ifndef BEAM_DECODER_EXCEPTION_HPP
#define BEAM_DECODER_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Signals that an error occurred while decoding. */
  class DecoderException : public std::runtime_error {
    public:
      using runtime_error::runtime_error;

      /** Constructs a DecoderException. */
      DecoderException();
  };

  inline DecoderException::DecoderException()
    : DecoderException("Failed to decode.") {}
}

#endif
