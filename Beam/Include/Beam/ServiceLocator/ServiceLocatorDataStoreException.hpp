#ifndef BEAM_SERVICE_LOCATOR_DATA_STORE_EXCEPTION_HPP
#define BEAM_SERVICE_LOCATOR_DATA_STORE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam {

  /** Exception to indicate an operation on a ServiceLocatorDataStore failed. */
  class ServiceLocatorDataStoreException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs a ServiceLocatorDataStoreException. */
      ServiceLocatorDataStoreException();
  };

  inline ServiceLocatorDataStoreException::ServiceLocatorDataStoreException()
    : ServiceLocatorDataStoreException("Operation failed") {}
}

#endif
