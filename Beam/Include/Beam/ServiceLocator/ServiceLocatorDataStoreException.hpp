#ifndef BEAM_SERVICE_LOCATOR_DATA_STORE_EXCEPTION_HPP
#define BEAM_SERVICE_LOCATOR_DATA_STORE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Exception to indicate an operation on a ServiceLocatorDataStore failed. */
  class ServiceLocatorDataStoreException : public IO::IOException {
    public:
      using IO::IOException::IOException;

      /** Constructs a ServiceLocatorDataStoreException. */
      ServiceLocatorDataStoreException();
  };

  inline ServiceLocatorDataStoreException::ServiceLocatorDataStoreException()
    : IO::IOException("Operation failed") {}
}

#endif
