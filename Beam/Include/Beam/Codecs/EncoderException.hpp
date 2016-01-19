#ifndef BEAM_ENCODEREXCEPTION_HPP
#define BEAM_ENCODEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Codecs/Codecs.hpp"

namespace Beam {
namespace Codecs {

  /*! \class EncoderException
      \brief Signals that an error occurred while encoding.
   */
  class EncoderException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs an EncoderException.
      /*!
        \param message A message describing the error.
      */
      EncoderException(const std::string& message);
  };

  inline EncoderException::EncoderException(const std::string& message)
      : std::runtime_error(message) {}
}
}

#endif
