#ifndef BEAM_WEB_SESSION_DATA_STORE_EXCEPTION_HPP
#define BEAM_WEB_SESSION_DATA_STORE_EXCEPTION_HPP
#include "Beam/IO/IOException.hpp"

namespace Beam {

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

#endif
