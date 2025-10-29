#ifndef BEAM_SERVICE_PROTOCOL_SERVLET_HPP
#define BEAM_SERVICE_PROTOCOL_SERVLET_HPP
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam {

  /** Determines whether a type is a valid ServiceProtocolServlet. */
  template<typename T>
  concept IsServiceProtocolServlet = requires {
    typename T::Container;
  } && requires(T& servlet) {
    { servlet.register_services(std::declval<Out<ServiceSlots<typename
        T::Container::ServiceProtocolServer::ServiceProtocolClient>>>())
    } -> std::same_as<void>;
    { servlet.close() } -> std::same_as<void>;
  };

  /**
   * Specifies the handlers to a set of services.
   * @tparam C The type of ServiceProtocolServletContainer instantiating the
   *        Servlet.
   */
  template<typename C>
  class ServiceProtocolServlet {
    public:

      /**
       * The type of ServiceProtocolServletContainer instantiating the Servlet.
       */
      using Container = C;

      /**
       * Registers the service handlers.
       * @param slots Stores the service slots.
       */
      void register_services(Out<ServiceSlots<
        typename Container::ServiceProtocolServer::ServiceProtocolClient>>
          slots);

      /**
       * Handler for when a ServiceProtocolClient has been accepted.
       * @param client The ServiceProtocolClient that was accepted.
       */
      void handle_accept(typename Container::ServiceProtocolClient& client);

      /**
       * Handler for when a ServiceProtocolClient has been closed.
       * @param client The ServiceProtocolClient that was closed.
       */
      void handle_close(typename Container::ServiceProtocolClient& client);

      void close();
  };
}

#endif
