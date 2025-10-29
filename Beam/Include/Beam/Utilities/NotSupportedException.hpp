#ifndef BEAM_NOT_SUPPORTED_EXCEPTION_HPP
#define BEAM_NOT_SUPPORTED_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Indicates that an optional virtual method is not supported. */
  class NotSupportedException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
