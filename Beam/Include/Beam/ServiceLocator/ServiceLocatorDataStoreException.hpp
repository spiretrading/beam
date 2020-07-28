#ifndef BEAM_SERVICE_LOCATOR_DATA_STORE_EXCEPTION_HPP
#define BEAM_SERVICE_LOCATOR_DATA_STORE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Exception to indicate an operation on a ServiceLocatorDataStore failed. */
  class ServiceLocatorDataStoreException : public IO::IOException {
    public:

      /** Constructs a ServiceLocatorDataStoreException. */
      ServiceLocatorDataStoreException();

      /**
       * Constructs a ServiceLocatorDataStoreException.
       * @param message A message describing the error.
       */
      explicit ServiceLocatorDataStoreException(const std::string& message);
  };

  inline ServiceLocatorDataStoreException::ServiceLocatorDataStoreException()
    : IO::IOException("Operation failed") {}

  inline ServiceLocatorDataStoreException::ServiceLocatorDataStoreException(
    const std::string& message)
    : IO::IOException(message) {}
}

#endif
