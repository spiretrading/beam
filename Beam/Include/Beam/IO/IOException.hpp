#ifndef BEAM_IO_EXCEPTION_HPP
#define BEAM_IO_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Signals that an error occurred while performing an IO operation. */
  class IOException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an IOException. */
      IOException();
  };

  inline IOException::IOException()
    : IOException("IO operation failed.") {}
}

#endif
