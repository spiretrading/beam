#ifndef BEAM_UIDSERVICEEXCEPTION_HPP
#define BEAM_UIDSERVICEEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/UidService/UidService.hpp"

namespace Beam {
namespace UidService {

  /*! \class UidServiceException
      \brief Indicates an exception in the UidServer or UidClient.
   */
  class UidServiceException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a UidServiceException.
      /*!
        \param message A message describing the error.
      */
      UidServiceException(const std::string& message);

      virtual ~UidServiceException() throw();
  };

  inline UidServiceException::UidServiceException(const std::string& message)
      : std::runtime_error(message) {}

  inline UidServiceException::~UidServiceException() throw() {}
}
}

#endif
