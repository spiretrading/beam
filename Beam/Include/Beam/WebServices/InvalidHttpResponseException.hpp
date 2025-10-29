#ifndef BEAM_INVALID_HTTP_RESPONSE_EXCEPTION_HPP
#define BEAM_INVALID_HTTP_RESPONSE_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

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

#endif
