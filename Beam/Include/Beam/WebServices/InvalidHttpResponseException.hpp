#ifndef BEAM_INVALIDHTTPRESPONSEEXCEPTION_HPP
#define BEAM_INVALIDHTTPRESPONSEEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class InvalidHttpResponseException
      \brief Signals an invalid HTTP response.
   */
  class InvalidHttpResponseException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an InvalidHttpResponseException.
      InvalidHttpResponseException();

      //! Constructs an InvalidHttpResponseException.
      /*!
        \param message A message describing the error.
      */
      InvalidHttpResponseException(const std::string& message);

      virtual ~InvalidHttpResponseException() throw();
  };

  inline InvalidHttpResponseException::InvalidHttpResponseException()
      : InvalidHttpResponseException{"Invalid HTTP Response."} {}

  inline InvalidHttpResponseException::InvalidHttpResponseException(
      const std::string& message)
      : std::runtime_error{message} {}

  inline InvalidHttpResponseException::
      ~InvalidHttpResponseException() throw() {}
}
}

#endif
