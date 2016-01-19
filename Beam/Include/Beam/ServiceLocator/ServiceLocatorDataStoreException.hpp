#ifndef BEAM_SERVICELOCATORDATASTOREEXCEPTION_HPP
#define BEAM_SERVICELOCATORDATASTOREEXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class ServiceLocatorDataStoreException
      \brief Exception to indicate an operation on a ServiceLocatorDataStore
             failed.
   */
  class ServiceLocatorDataStoreException : public IO::IOException {
    public:

      //! Constructs a ServiceLocatorDataStoreException.
      ServiceLocatorDataStoreException();

      //! Constructs a ServiceLocatorDataStoreException.
      /*!
        \param message A message describing the error.
      */
      ServiceLocatorDataStoreException(const std::string& message);

      virtual ~ServiceLocatorDataStoreException() throw();
  };

  inline ServiceLocatorDataStoreException::ServiceLocatorDataStoreException()
      : IO::IOException("Operation failed") {}

  inline ServiceLocatorDataStoreException::ServiceLocatorDataStoreException(
      const std::string& message)
      : IO::IOException(message) {}

  inline ServiceLocatorDataStoreException::~ServiceLocatorDataStoreException()
      throw() {}
}
}

#endif
