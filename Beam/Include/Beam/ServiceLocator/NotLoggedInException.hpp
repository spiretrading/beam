#ifndef BEAM_NOTLOGGEDINEXCEPTION_HPP
#define BEAM_NOTLOGGEDINEXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class NotLoggedInException
      \brief An service was requested without being logged in.
   */
  class NotLoggedInException : public IO::IOException {
    public:

      //! Constructs a NotLoggedInException.
      NotLoggedInException();

      //! Constructs a NotLoggedInException.
      /*!
        \param message A message describing the error.
      */
      NotLoggedInException(const std::string& message);

      virtual ~NotLoggedInException() throw();
  };

  inline NotLoggedInException::NotLoggedInException()
      : IO::IOException("Client is not logged in.") {}

  inline NotLoggedInException::NotLoggedInException(const std::string& message)
      : IO::IOException(message) {}

  inline NotLoggedInException::~NotLoggedInException() throw() {}
}
}

#endif
