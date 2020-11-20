#ifndef BEAM_END_OF_FILE_EXCEPTION_HPP
#define BEAM_END_OF_FILE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam::IO {

  /** Signals a file/resource has reached its end or has been closed. */
  class EndOfFileException : public IOException {
    public:

      /** Constructs an EndOfFileException. */
      EndOfFileException();

      /**
       * Constructs an EndOfFileException.
       * @param message A message describing the error.
       */
      EndOfFileException(const std::string& message);
  };

  inline EndOfFileException::EndOfFileException()
    : IOException("End of file.") {}

  inline EndOfFileException::EndOfFileException(const std::string& message)
    : IOException(message) {}
}

#endif
