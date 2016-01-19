#ifndef BEAM_SERVICEREQUESTEXCEPTION_HPP
#define BEAM_SERVICEREQUESTEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Serialization/Serialization.hpp"
#include "Beam/Services/Services.hpp"

namespace Beam {
namespace Services {

  /*! \class ServiceRequestException
      \brief Indicates a general failure to service a request.
   */
  class ServiceRequestException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a ServiceRequestException.
      ServiceRequestException();

      //! Constructs a ServiceRequestException.
      /*!
        \param message A message describing the error.
      */
      ServiceRequestException(const std::string& message);

      virtual ~ServiceRequestException() throw();

      virtual void Throw() const;

      virtual const char* what() const throw();

    protected:
      friend struct Serialization::DataShuttle;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      std::string m_message;
  };

  inline ServiceRequestException::ServiceRequestException()
      : std::runtime_error("") {}

  inline ServiceRequestException::ServiceRequestException(
      const std::string& message)
      : std::runtime_error(""),
        m_message(message) {}

  inline ServiceRequestException::~ServiceRequestException() throw() {}

  inline void ServiceRequestException::Throw() const {
    throw *this;
  }

  inline const char* ServiceRequestException::what() const throw() {
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
}

#endif
