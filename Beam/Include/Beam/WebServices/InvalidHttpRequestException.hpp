#ifndef AVALON_INVALIDHTTPREQUESTEXCEPTION_HPP
#define AVALON_INVALIDHTTPREQUESTEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Avalon/WebServices/WebServices.hpp"

namespace Avalon {
namespace WebServices {

  /*! \class InvalidHttpRequestException
      \brief Signals an invalid HTTP request.
   */
  class InvalidHttpRequestException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an InvalidHttpRequestException.
      /*!
        \param message A message describing the error.
      */
      InvalidHttpRequestException(const std::string& message);

      virtual ~InvalidHttpRequestException() throw();
  };
}
}

#endif // AVALON_INVALIDHTTPREQUESTEXCEPTION_HPP
