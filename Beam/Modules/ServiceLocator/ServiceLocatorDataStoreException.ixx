module;
#include "Prelude.hpp"

export module Beam:ServiceLocatorDataStoreException;

export namespace Beam {

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

