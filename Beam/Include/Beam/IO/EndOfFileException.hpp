#ifndef BEAM_ENDOFFILEEXCEPTION_HPP
#define BEAM_ENDOFFILEEXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam {
namespace IO {

  /*! \class EndOfFileException
      \brief Indicates a Reader has reached the end of file or stream.
   */
  class EndOfFileException : public IOException {
    public:

      //! Constructs an EndOfFileException.
      EndOfFileException();

      //! Constructs an EndOfFileException.
      /*!
        \param message A message describing the error.
      */
      EndOfFileException(const std::string& message);
  };

  inline EndOfFileException::EndOfFileException()
    : IOException("End of file.") {}

  inline EndOfFileException::EndOfFileException(const std::string& message)
    : IOException(message) {}
}
}

#endif
