#ifndef BEAM_SERVICE_REQUEST_EXCEPTION_HPP
#define BEAM_SERVICE_REQUEST_EXCEPTION_HPP
#include <concepts>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include "Beam/IO/IOException.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** Indicates a general failure to service a request. */
  class ServiceRequestException : public std::runtime_error {
    public:

      /** Constructs a ServiceRequestException. */
      ServiceRequestException();

      /**
       * Constructs a ServiceRequestException.
       * @param message The reason for the exception.
       */
      explicit ServiceRequestException(const std::string& message);

      /**
       * Throws this exception.
       * @throws ServiceRequestException Always.
       */
      virtual void raise() const;

      const char* what() const noexcept override;

    protected:
      friend struct Beam::DataShuttle;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      std::string m_message;
  };

  /**
   * When called from within an exception handler/catch clause involving a
   * service, rethrows an appropriate nested exception with a specified message.
   * @param message The message to include in the nested exception.
   */
  inline void rethrow_nested_service_exception(const std::string& message) {
    try {
      std::rethrow_exception(std::current_exception());
    } catch(const IOException&) {
      std::throw_with_nested(IOException(message));
    } catch(const ServiceRequestException&) {
      std::throw_with_nested(ServiceRequestException(message));
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
  inline std::exception_ptr make_nested_service_exception(
      const std::string& message) {
    try {
      rethrow_nested_service_exception(message);
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
  template<std::invocable<> F>
  decltype(auto) service_or_throw_with_nested(
      F&& f, const std::string& message) {
    try {
      return std::forward<F>(f)();
    } catch(const std::exception&) {
      rethrow_nested_service_exception(message);
      throw;
    }
  }

  inline ServiceRequestException::ServiceRequestException()
    : std::runtime_error("Service request failed.") {}

  inline ServiceRequestException::ServiceRequestException(
    const std::string& message)
    : std::runtime_error(message),
      m_message(message) {}

  inline void ServiceRequestException::raise() const {
    boost::throw_with_location(*this);
  }

  inline const char* ServiceRequestException::what() const noexcept {
    return m_message.c_str();
  }

  template<IsShuttle S>
  void ServiceRequestException::shuttle(S& shuttle, unsigned int version) {
    if(version == 0) {
      shuttle.shuttle("message", m_message);
    }
  }
}

#endif
