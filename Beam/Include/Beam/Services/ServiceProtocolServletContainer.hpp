#ifndef BEAM_SERVICEPROTOCOLSERVLETCONTAINER_HPP
#define BEAM_SERVICEPROTOCOLSERVLETCONTAINER_HPP
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Services/ServiceProtocolServletContainerDetails.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Services {

  /*! \class ServiceProtocolServletContainer
      \brief Executes and manages a ServiceProtocolServlet.
      \tparam MetaServlet The type of ServiceProtocolServlet to host.
      \tparam ServerConnectionType The type of ServerConnection accepting
              Channels.
      \tparam SenderType The type of Sender used for serialization.
      \tparam EncoderType The type of Encoder used for messages.
      \tparam TimerType The type of Timer used for heartbeats.
      \tparam ServletPointerPolicy The type of pointer to use with the Servlet.
   */
  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy = LocalPointerPolicy>
  class ServiceProtocolServletContainer {
    public:

      //! The type of ServiceProtocolServlet to host.
      using Servlet = typename MetaServlet::template apply<
        ServiceProtocolServletContainer>::type;

      //! The type of ServerConnection accepting Channels.
      using ServerConnection = GetTryDereferenceType<ServerConnectionType>;

      //! The type of Sender used for serialization.
      using Sender = SenderType;

      //! The type of Encoder used for messages.
      using Encoder = EncoderType;

      //! The type of Timer used for heartbeats.
      using Timer = GetTryDereferenceType<TimerType>;

      //! The type of ServiceProtocolServer.
      using ServiceProtocolServer = Services::ServiceProtocolServer<
        ServerConnectionType, Sender, Encoder, TimerType,
        typename MetaServlet::Session, SupportsParallelism<MetaServlet>::value>;

      //! The type of ServiceProtocolClient.
      using ServiceProtocolClient =
        typename ServiceProtocolServer::ServiceProtocolClient;

      //! Constructs the ServiceProtocolServletContainer.
      /*!
        \param servlet Initializes the Servlet.
        \param serverConnection Accepts connections to the servlet.
        \param timerFactory The type of Timer used for heartbeats.
      */
      template<typename ServletForward, typename ServerConnectionForward>
      ServiceProtocolServletContainer(ServletForward&& servlet,
        ServerConnectionForward&& serverConnection,
        const typename ServiceProtocolServer::TimerFactory& timerFactory);

      ~ServiceProtocolServletContainer();

      void Open();

      void Close();

    private:
      typename ServletPointerPolicy::template apply<Servlet>::type m_servlet;
      ServiceProtocolServer m_protocolServer;

      void OnClientAccepted(ServiceProtocolClient& client);
      void OnClientClosed(ServiceProtocolClient& client);
  };

  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy>
  template<typename ServletForward, typename ServerConnectionForward>
  ServiceProtocolServletContainer<MetaServlet, ServerConnectionType,
      SenderType, EncoderType, TimerType, ServletPointerPolicy>::
      ServiceProtocolServletContainer(ServletForward&& servlet,
      ServerConnectionForward&& serverConnection,
      const typename ServiceProtocolServer::TimerFactory& timerFactory)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_servlet{std::forward<ServletForward>(servlet)},
        m_protocolServer{std::forward<ServerConnectionForward>(
          serverConnection), timerFactory,
          std::bind(&ServiceProtocolServletContainer::OnClientAccepted, this,
          std::placeholders::_1), std::bind(
          &ServiceProtocolServletContainer::OnClientClosed, this,
          std::placeholders::_1)} {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_servlet->RegisterServices(Store(m_protocolServer.GetSlots()));
  }

  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy>
  ServiceProtocolServletContainer<MetaServlet, ServerConnectionType,
      SenderType, EncoderType, TimerType, ServletPointerPolicy>::
      ~ServiceProtocolServletContainer() {
    Close();
  }

  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy>
  void ServiceProtocolServletContainer<MetaServlet, ServerConnectionType,
      SenderType, EncoderType, TimerType, ServletPointerPolicy>::Open() {
    m_servlet->Open();
    m_protocolServer.Open();
  }

  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy>
  void ServiceProtocolServletContainer<MetaServlet, ServerConnectionType,
      SenderType, EncoderType, TimerType, ServletPointerPolicy>::Close() {
    m_protocolServer.Close();
    m_servlet->Close();
  }

  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy>
  void ServiceProtocolServletContainer<MetaServlet, ServerConnectionType,
      SenderType, EncoderType, TimerType, ServletPointerPolicy>::
      OnClientAccepted(ServiceProtocolClient& client) {
    Details::InvokeClientAccepted<Details::HasClientAcceptedMethod<Servlet,
      ServiceProtocolClient>::value>()(*m_servlet, client);
  }

  template<typename MetaServlet, typename ServerConnectionType,
    typename SenderType, typename EncoderType, typename TimerType,
    typename ServletPointerPolicy>
  void ServiceProtocolServletContainer<MetaServlet, ServerConnectionType,
      SenderType, EncoderType, TimerType, ServletPointerPolicy>::OnClientClosed(
      ServiceProtocolClient& client) {
    Details::InvokeClientClosed<Details::HasClientClosedMethod<Servlet,
      ServiceProtocolClient>::value>()(*m_servlet, client);
  }
}
}

#endif
