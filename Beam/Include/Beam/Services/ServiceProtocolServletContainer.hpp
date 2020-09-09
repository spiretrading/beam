#ifndef BEAM_SERVICEPROTOCOLSERVLETCONTAINER_HPP
#define BEAM_SERVICEPROTOCOLSERVLETCONTAINER_HPP
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Services/ServiceProtocolServletContainerDetails.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Services {

  /*! \class ServiceProtocolServletContainer
      \brief Executes and manages a ServiceProtocolServlet.
      \tparam M The type of ServiceProtocolServlet to host.
      \tparam C The type of ServerConnection accepting Channels.
      \tparam S The type of Sender used for serialization.
      \tparam E The type of Encoder used for messages.
      \tparam T The type of Timer used for heartbeats.
      \tparam P The type of pointer to use with the Servlet.
   */
  template<typename M, typename C, typename S, typename E, typename T,
    typename P = LocalPointerPolicy>
  class ServiceProtocolServletContainer {
    public:

      //! The type of ServiceProtocolServlet to host.
      using Servlet = typename M::template apply<
        ServiceProtocolServletContainer>::type;

      //! The type of ServerConnection accepting Channels.
      using ServerConnection = GetTryDereferenceType<C>;

      //! The type of Sender used for serialization.
      using Sender = S;

      //! The type of Encoder used for messages.
      using Encoder = E;

      //! The type of Timer used for heartbeats.
      using Timer = GetTryDereferenceType<T>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer = Services::ServiceProtocolServer<
        C, Sender, Encoder, T, typename M::Session,
        SupportsParallelism<M>::value>;

      //! The type of ServiceProtocolClient.
      using ServiceProtocolClient =
        typename ServiceProtocolServer::ServiceProtocolClient;

      //! Constructs the ServiceProtocolServletContainer.
      /*!
        \param servlet Initializes the Servlet.
        \param serverConnection Accepts connections to the servlet.
        \param timerFactory The type of Timer used for heartbeats.
      */
      template<typename SF, typename CF>
      ServiceProtocolServletContainer(SF&& servlet, CF&& serverConnection,
        typename ServiceProtocolServer::TimerFactory timerFactory);

      ~ServiceProtocolServletContainer();

      void Close();

    private:
      typename P::template apply<Servlet>::type m_servlet;
      Routines::Async<void> m_isOpen;
      ServiceProtocolServer m_protocolServer;

      ServiceProtocolServletContainer(
        const ServiceProtocolServletContainer&) = delete;
      ServiceProtocolServletContainer& operator =(
        const ServiceProtocolServletContainer&) = delete;
      void OnClientAccepted(ServiceProtocolClient& client);
      void OnClientClosed(ServiceProtocolClient& client);
  };

  template<typename M, typename C, typename S, typename E, typename T,
    typename P>
  template<typename SF, typename CF>
  ServiceProtocolServletContainer<M, C, S, E, T, P>::
      ServiceProtocolServletContainer(SF&& servlet, CF&& serverConnection,
      typename ServiceProtocolServer::TimerFactory timerFactory)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_servlet(std::forward<SF>(servlet)),
        m_protocolServer(std::forward<CF>(serverConnection),
          std::move(timerFactory),
          std::bind(&ServiceProtocolServletContainer::OnClientAccepted, this,
          std::placeholders::_1), std::bind(
          &ServiceProtocolServletContainer::OnClientClosed, this,
          std::placeholders::_1)) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_servlet->RegisterServices(Store(m_protocolServer.GetSlots()));
    m_isOpen.GetEval().SetResult();
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P>
  ServiceProtocolServletContainer<M, C, S, E, T, P>::
      ~ServiceProtocolServletContainer() {
    Close();
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P>
  void ServiceProtocolServletContainer<M, C, S, E, T, P>::Close() {
    m_protocolServer.Close();
    m_servlet->Close();
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P>
  void ServiceProtocolServletContainer<M, C, S, E, T, P>::OnClientAccepted(
      ServiceProtocolClient& client) {
    m_isOpen.Get();
    Details::InvokeClientAccepted<Details::HasClientAcceptedMethod<Servlet,
      ServiceProtocolClient>::value>()(*m_servlet, client);
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P>
  void ServiceProtocolServletContainer<M, C, S, E, T, P>::OnClientClosed(
      ServiceProtocolClient& client) {
    Details::InvokeClientClosed<Details::HasClientClosedMethod<Servlet,
      ServiceProtocolClient>::value>()(*m_servlet, client);
  }
}

#endif
