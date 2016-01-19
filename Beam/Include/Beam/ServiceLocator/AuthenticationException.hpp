#ifndef BEAM_AUTHENTICATIONEXCEPTION_HPP
#define BEAM_AUTHENTICATIONEXCEPTION_HPP
#include "Beam/IO/ConnectException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class AuthenticationException
      \brief Signals an authentication error during a connect operation.
   */
  class AuthenticationException : public IO::ConnectException {
    public:

      //! Constructs an AuthenticationException.
      AuthenticationException();

      //! Constructs an AuthenticationException.
      /*!
        \param message A message describing the error.
      */
      AuthenticationException(const std::string& message);

      virtual ~AuthenticationException() throw();
  };

  inline AuthenticationException::AuthenticationException()
      : ConnectException("Unable to authenticate connection.") {}

  inline AuthenticationException::AuthenticationException(
      const std::string& message)
      : ConnectException(message) {}

  inline AuthenticationException::~AuthenticationException() throw() {}
}
}

#endif
