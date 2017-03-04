#ifndef BEAM_HTTPSERVLETCONTAINER_HPP
#define BEAM_HTTPSERVLETCONTAINER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"
#include "Beam/WebServices/HttpServer.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {
namespace Details {
  BEAM_DEFINE_HAS_METHOD(IsServletClass, Open, void);
  BEAM_DEFINE_HAS_METHOD(HasWebSocketSlots, GetWebSocketSlots, void);
  template<typename ContainerType, typename ServletType,
    bool dummy = IsServletClass<ServletType>::value>
  struct GetServletHelper;

  template<typename ContainerType, typename ServletType>
  struct GetServletHelper<ContainerType, ServletType, true> {
    using type = ServletType;
  };

  template<typename ContainerType, typename ServletType>
  struct GetServletHelper<ContainerType, ServletType, false> {
    using type = typename ServletType::template apply<ContainerType>::type;
  };

  template<typename ContainerType, typename ServletType>
  using GetServlet = typename GetServletHelper<
    ContainerType, ServletType>::type;

  template<typename Container, typename Servlet>
  auto GetWebSocketSlots(Servlet& servlet) ->
      decltype(servlet.GetWebSocketSlots()) {
    return servlet.GetWebSocketSlots();
  }

  template<typename Container, typename Servlet>
  auto GetWebSocketSlots(Servlet& servlet) {
    return std::vector<Container::HttpServer::HttpUpgradeSlot>();
  }
}

  /*! \class HttpServletContainer
      \brief Composes an HttpServlet with an HttpServer.
      \tparam ServletType The type of HttpServlet servicing HTTP requests.
      \tparam ServerConnectionType The type of ServerConnection accepting
              Channels.
   */
  template<typename ServletType, typename ServerConnectionType>
  class HttpServletContainer : private boost::noncopyable {
    public:

      //! The type of HttpServlet servicing HTTP requests.
      using Servlet = Details::GetServlet<HttpServletContainer, ServletType>;

      //! The type of ServerConnection accepting Channels.
      using ServerConnection = GetTryDereferenceType<ServerConnectionType>;

      //! The type of HttpServer used.
      using HttpServer = Beam::WebServices::HttpServer<ServerConnectionType>;

      //! Constructs the HttpServletContainer.
      /*!
        \param servlet Initializes the Servlet.
        \param serverConnection Accepts connections to the servlet.
      */
      template<typename ServletForward, typename ServerConnectionForward>
      HttpServletContainer(ServletForward&& servlet,
        ServerConnectionForward&& serverConnection);

      ~HttpServletContainer();

      void Open();

      void Close();

    private:
      Beam::LocalPtr<Servlet> m_servlet;
      Beam::LocalPtr<HttpServer> m_server;
  };

  template<typename ServletType, typename ServerConnectionType>
  template<typename ServletForward, typename ServerConnectionForward>
  HttpServletContainer<ServletType, ServerConnectionType>::HttpServletContainer(
      ServletForward&& servlet, ServerConnectionForward&& serverConnection)
      : m_servlet{std::forward<ServletForward>(servlet)},
        m_server{std::forward<ServerConnectionForward>(serverConnection),
          m_servlet->GetSlots(),
          Details::GetWebSocketSlots<HttpServletContainer>(*m_servlet)} {}

  template<typename ServletType, typename ServerConnectionType>
  HttpServletContainer<ServletType, ServerConnectionType>::
      ~HttpServletContainer() {
    Close();
  }

  template<typename ServletType, typename ServerConnectionType>
  void HttpServletContainer<ServletType, ServerConnectionType>::Open() {
    m_servlet->Open();
    m_server->Open();
  }

  template<typename ServletType, typename ServerConnectionType>
  void HttpServletContainer<ServletType, ServerConnectionType>::Close() {
    m_server->Close();
    m_servlet->Close();
  }
}
}

#endif
