#ifndef BEAM_REGISTRYDATASTOREEXCEPTION_HPP
#define BEAM_REGISTRYDATASTOREEXCEPTION_HPP
#include "Beam/IO/IOException.hpp"
#include "Beam/RegistryService/RegistryService.hpp"

namespace Beam {
namespace RegistryService {

  /*! \class RegistryDataStoreException
      \brief Exception to indicate an operation on a RegistryDataStore failed.
   */
  class RegistryDataStoreException : public IO::IOException {
    public:

      //! Constructs a RegistryDataStoreException.
      RegistryDataStoreException();

      //! Constructs a RegistryDataStoreException.
      /*!
        \param message A message describing the error.
      */
      RegistryDataStoreException(const std::string& message);

      virtual ~RegistryDataStoreException() throw();
  };

  inline RegistryDataStoreException::RegistryDataStoreException()
      : IO::IOException("Operation failed") {}

  inline RegistryDataStoreException::RegistryDataStoreException(
      const std::string& message)
      : IO::IOException(message) {}

  inline RegistryDataStoreException::~RegistryDataStoreException() throw() {}
}
}

#endif
