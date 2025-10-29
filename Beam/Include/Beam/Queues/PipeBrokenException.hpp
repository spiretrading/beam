#ifndef BEAM_PIPE_BROKEN_EXCEPTION_HPP
#define BEAM_PIPE_BROKEN_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Thrown when an operation is performed on a broken Queue. */
  class PipeBrokenException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a PipeBrokenException. */
      PipeBrokenException();
  };

  inline PipeBrokenException::PipeBrokenException()
    : PipeBrokenException("Pipe broken.") {}
}

#endif
