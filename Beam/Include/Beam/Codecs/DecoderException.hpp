#ifndef BEAM_DECODER_EXCEPTION_HPP
#define BEAM_DECODER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Codecs/Codecs.hpp"

namespace Beam::Codecs {

  /** Signals that an error occurred while decoding. */
  class DecoderException : public std::runtime_error, public boost::exception {
    public:
      using runtime_error::runtime_error;

      /** Constructs a DecoderException. */
      DecoderException();
  };

  inline DecoderException::DecoderException()
    : DecoderException("Failed to decode.") {}
}

#endif
