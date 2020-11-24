#ifndef BEAM_SERVICE_PROTOCOL_SERVLET_HPP
#define BEAM_SERVICE_PROTOCOL_SERVLET_HPP
#include "Beam/Serialization/TypeRegistry.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam::Services {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(HasContainerType, Container);
}

  /**
   * Specifies the handlers to a set of Services.
   * @param <C> The type of ServiceProtocolServletContainer instantiating the
   *        Servlet.
   */
  template<typename C>
  struct ServiceProtocolServlet : Concept<ServiceProtocolServlet<C>> {

    /**
     * The type of ServiceProtocolServletContainer instantiating the Servlet.
     */
    using Container = C;

    /**
     * Registers the service handlers.
     * @param slots Stores the service slots.
     */
    void RegisterServices(Out<ServiceSlots<
      typename Container::ServiceProtocolServer::ServiceProtocolClient>> slots);

    /** Handler for when a ServiceProtocolClient has been accepted. */
    void HandleClientAccepted(
      typename Container::ServiceProtocolClient& client);

    /** Handler for when a ServiceProtocolClient has been closed. */
    void HandleClientClosed(typename Container::ServiceProtocolClient& client);

    void Close();
  };

  /**
   * Tests whether a type satisfies some particular ServiceProtocolServlet
   * Concept.
   * @param <T> The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsServiceProtocolServlet : std::false_type {};

  template<typename T>
  struct IsServiceProtocolServlet<T, std::enable_if_t<
    Details::HasContainerType<T>::value>> : ImplementsConcept<
    T, ServiceProtocolServlet<typename T::Container>>::type {};
}

#endif
