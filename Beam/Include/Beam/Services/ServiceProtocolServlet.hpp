#ifndef BEAM_SERVICEPROTOCOLSERVLET_HPP
#define BEAM_SERVICEPROTOCOLSERVLET_HPP
#include "Beam/Serialization/TypeRegistry.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace Services {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(HasContainerType, Container);
}

  /*! \class ServiceProtocolServlet
      \brief Specifies the handlers to a set of Services.
      \tparam ContainerType The type of ServiceProtocolServletContainer
                            instantiating the Servlet.
   */
  template<typename ContainerType>
  struct ServiceProtocolServlet :
      Concept<ServiceProtocolServlet<ContainerType>> {

    //! The type of ServiceProtocolServletContainer instantiating the Servlet.
    using Container = ContainerType;

    //! Registers the service handlers.
    /*!
      \param slots Stores the service slots.
    */
    void RegisterServices(Out<ServiceSlots<
      typename Container::ServiceProtocolServer::ServiceProtocolClient>> slots);

    //! Handler for when a ServiceProtocolClient has been accepted.
    void HandleClientAccepted(
      typename Container::ServiceProtocolClient& client);

    //! Handler for when a ServiceProtocolClient has been closed.
    void HandleClientClosed(typename Container::ServiceProtocolClient& client);

    void Close();
  };

  /*! \struct IsServiceProtocolServlet
      \brief Tests whether a type satisfies some particular
             ServiceProtocolServlet Concept.
      \tparam T The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsServiceProtocolServlet : std::false_type {};

  template<typename T>
  struct IsServiceProtocolServlet<T, typename std::enable_if<
    Details::HasContainerType<T>::value>::type>
    : boost::mpl::if_c<ImplementsConcept<T, ServiceProtocolServlet<
      typename T::Container>>::value, std::true_type, std::false_type>::type {};
}
}

#endif
