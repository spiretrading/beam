#ifndef BEAM_DECODEREXCEPTION_HPP
#define BEAM_DECODEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Codecs/Codecs.hpp"

namespace Beam {
namespace Codecs {

  /*! \class DecoderException
      \brief Signals that an error occurred while decoding.
   */
  class DecoderException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs a DecoderException.
      /*!
        \param message A message describing the error.
      */
      DecoderException(const std::string& message);
  };

  inline DecoderException::DecoderException(const std::string& message)
      : std::runtime_error(message) {}
}
}

#endif
