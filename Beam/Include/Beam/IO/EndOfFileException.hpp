#ifndef BEAM_END_OF_FILE_EXCEPTION_HPP
#define BEAM_END_OF_FILE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam::IO {

  /** Signals a file/resource has reached its end or has been closed. */
  class EndOfFileException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs an EndOfFileException. */
      EndOfFileException();
  };

  inline EndOfFileException::EndOfFileException()
    : IOException("End of file.") {}
}

#endif
