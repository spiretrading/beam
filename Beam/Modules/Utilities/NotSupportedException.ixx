module;
#include "Prelude.hpp"

export module Beam:NotSupportedException;

export namespace Beam {

  /** Indicates that an optional virtual method is not supported. */
  class NotSupportedException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

