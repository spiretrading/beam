module;
#include "Prelude.hpp"

export module Beam:WebSessionDataStoreException;

export namespace Beam {

  /** Exception to indicate an operation on a WebSessionDataStore failed. */
  class WebSessionDataStoreException : public IOException {
    public:
      using IOException::IOException;

      /** Constructs a WebSessionDataStoreException. */
      WebSessionDataStoreException();
  };

  inline WebSessionDataStoreException::WebSessionDataStoreException()
    : WebSessionDataStoreException("Operation failed.") {}
}

