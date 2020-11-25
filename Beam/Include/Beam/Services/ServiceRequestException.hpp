#ifndef BEAM_SERVICE_REQUEST_EXCEPTION_HPP
#define BEAM_SERVICE_REQUEST_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/IO/IOException.hpp"
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

  /**
   * When called from within an exception handler/catch clause involving a
   * service, rethrows an appropriate nested exception with a specified message.
   * @param message The message to include in the nested exception.
   */
  inline void RethrowNestedServiceException(const std::string& message) {
    try {
      std::rethrow_exception(std::current_exception());
    } catch(const IO::IOException&) {
      std::throw_with_nested(IO::IOException(message));
    } catch(const Services::ServiceRequestException&) {
      std::throw_with_nested(Services::ServiceRequestException(message));
    } catch(const std::exception&) {
      std::throw_with_nested(std::runtime_error(message));
    }
  }

  /**
   * When called from within an exception handler/catch clause involving a
   * service, returns an appropriate nested exception with a specified message.
   * @param message The message to include in the nested exception.
   * @return A nested exception with the specified message.
   */
  inline std::exception_ptr MakeNestedServiceException(
      const std::string& message) {
    try {
      RethrowNestedServiceException(message);
      return nullptr;
    } catch(...) {
      return std::current_exception();
    }
  }

  /**
   * Invokes a function performing a service operation and throws an appropriate
   * nested exception with a specified message on failure.
   * @param f The function to invoke.
   * @param message The message to include in the nested exception.
   * @return The result of calling <i>f</i>.
   */
  template<typename F>
  decltype(auto) ServiceOrThrowWithNested(F&& f, const std::string& message) {
    try {
      return f();
    } catch(const std::exception&) {
      RethrowNestedServiceException(message);
      throw;
    }
  }

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
