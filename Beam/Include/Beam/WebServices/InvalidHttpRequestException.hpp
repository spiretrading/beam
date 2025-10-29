#ifndef BEAM_INVALID_HTTP_REQUEST_EXCEPTION_HPP
#define BEAM_INVALID_HTTP_REQUEST_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

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

#endif
