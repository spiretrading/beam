#ifndef BEAM_INVALIDHTTPREQUESTEXCEPTION_HPP
#define BEAM_INVALIDHTTPREQUESTEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class InvalidHttpRequestException
      \brief Signals an invalid HTTP request.
   */
  class InvalidHttpRequestException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an InvalidHttpRequestException.
      InvalidHttpRequestException();

      //! Constructs an InvalidHttpRequestException.
      /*!
        \param message A message describing the error.
      */
      InvalidHttpRequestException(const std::string& message);

      virtual ~InvalidHttpRequestException() throw();
  };

  inline InvalidHttpRequestException::InvalidHttpRequestException()
      : InvalidHttpRequestException{"Invalid HTTP Request."} {}

  inline InvalidHttpRequestException::InvalidHttpRequestException(
      const std::string& message)
      : std::runtime_error{message} {}

  inline InvalidHttpRequestException::~InvalidHttpRequestException() throw() {}
}
}

#endif
