#ifndef BEAM_SERVICE_REQUEST_EXCEPTION_HPP
#define BEAM_SERVICE_REQUEST_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Serialization/Serialization.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam::Services {

  /** Indicates a general failure to service a request. */
  class ServiceRequestException : public std::runtime_error,
      public boost::exception {
    public:

      /** Constructs a ServiceRequestException. */
      ServiceRequestException();

      /**
       * Constructs a ServiceRequestException.
       * @param message The reason for the exception.
       */
      ServiceRequestException(const std::string& message);

      virtual void Throw() const;

      const char* what() const noexcept override;

    protected:
      friend struct Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      std::string m_message;
  };

  inline ServiceRequestException::ServiceRequestException()
    : std::runtime_error("Service request failed.") {}

  inline ServiceRequestException::ServiceRequestException(
    const std::string& message)
    : std::runtime_error(message),
      m_message(message) {}

  inline void ServiceRequestException::Throw() const {
    throw *this;
  }

  inline const char* ServiceRequestException::what() const noexcept {
    return m_message.c_str();
  }

  template<typename Shuttler>
  void ServiceRequestException::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    if(version == 0) {
      shuttle.Shuttle("message", m_message);
    }
  }
}

#endif
