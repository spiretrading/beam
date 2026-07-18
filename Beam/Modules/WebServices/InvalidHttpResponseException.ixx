module;
#include "Prelude.hpp"

export module Beam:InvalidHttpResponseException;

export namespace Beam {

  /** Signals an invalid HTTP response. */
  class InvalidHttpResponseException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an InvalidHttpResponseException. */
      InvalidHttpResponseException();
  };

  inline InvalidHttpResponseException::InvalidHttpResponseException()
    : InvalidHttpResponseException("Invalid HTTP Response.") {}
}

