module;
#include "Prelude.hpp"

export module Beam:InvalidHttpRequestException;

export namespace Beam {

  /** Signals an invalid HTTP request. */
  class InvalidHttpRequestException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an InvalidHttpRequestException. */
      InvalidHttpRequestException();
  };

  inline InvalidHttpRequestException::InvalidHttpRequestException()
    : InvalidHttpRequestException("Invalid HTTP Request.") {}
}

