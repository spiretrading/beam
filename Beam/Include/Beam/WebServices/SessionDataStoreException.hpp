#ifndef BEAM_SESSION_DATA_STORE_EXCEPTION_HPP
#define BEAM_SESSION_DATA_STORE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class SessionDataStoreException
      \brief Exception to indicate an operation on a SessionDataStore failed.
   */
  class SessionDataStoreException : public IO::IOException {
    public:

      //! Constructs a SessionDataStoreException.
      SessionDataStoreException();

      //! Constructs a SessionDataStoreException.
      /*!
        \param message A message describing the error.
      */
      SessionDataStoreException(const std::string& message);

      virtual ~SessionDataStoreException() throw();
  };

  inline SessionDataStoreException::SessionDataStoreException()
      : IO::IOException{"Operation failed."} {}

  inline SessionDataStoreException::SessionDataStoreException(
      const std::string& message)
      : IO::IOException{message} {}

  inline SessionDataStoreException::~SessionDataStoreException() throw() {}
}
}

#endif
