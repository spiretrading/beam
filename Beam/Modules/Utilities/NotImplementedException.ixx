module;
#include "Prelude.hpp"

export module Beam:NotImplementedException;

export namespace Beam {

  /** Indicates that a function/method has not been implemented. */
  class NotImplementedException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a NotImplementedException. */
      NotImplementedException();
  };

  inline NotImplementedException::NotImplementedException()
    : NotImplementedException("Function is not implemented.") {}
}

