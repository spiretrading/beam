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

  template<typename T>
  struct HasSlots {
    template<typename C>
    static auto test(int) ->
      decltype(std::declval<C>().GetSlots(), std::true_type());

    template<typename>
    static std::false_type test(...);

    using type = decltype(test<T>(0));
    static const bool value = std::is_same<
      std::true_type, decltype(test<T>(0))>::value;
  };

  template<typename Servlet, bool dummy = HasSlots<Servlet>::value>
  struct GetSlotsHelper {};

  template<typename Servlet>
  struct GetSlotsHelper<Servlet, true> {
    auto operator ()(Servlet& servlet) {
      return servlet.GetSlots();
    }
  };

  template<typename Servlet>
  struct GetSlotsHelper<Servlet, false> {
    auto operator ()(Servlet& servlet) {
      return std::vector<HttpRequestSlot>();
    }
  };

  template<typename Servlet>
  auto GetSlots(Servlet& servlet) {
    return GetSlotsHelper<Servlet>()(servlet);
  }

  template<typename T>
  struct HasWebSocketSlots {
    template<typename C>
    static auto test(int) ->
      decltype(std::declval<C>().GetWebSocketSlots(), std::true_type());

    template<typename>
    static std::false_type test(...);

    using type = decltype(test<T>(0));
    static const bool value = std::is_same<
      std::true_type, decltype(test<T>(0))>::value;
  };

  template<typename Container, typename Servlet,
    bool dummy = HasWebSocketSlots<Servlet>::value>
  struct GetWebSocketsSlotsHelper {};

  template<typename Container, typename Servlet>
  struct GetWebSocketsSlotsHelper<Container, Servlet, true> {
    auto operator ()(Servlet& servlet) {
      return servlet.GetWebSocketSlots();
    }
  };

  template<typename Container, typename Servlet>
  struct GetWebSocketsSlotsHelper<Container, Servlet, false> {
    auto operator ()(Servlet& servlet) {
      return std::vector<typename Container::HttpServer::WebSocketSlot>();
    }
  };

  template<typename Container, typename Servlet>
  auto GetWebSocketSlots(Servlet& servlet) {
    return GetWebSocketsSlotsHelper<Container, Servlet>()(servlet);
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
      using HttpServer = ::Beam::WebServices::HttpServer<ServerConnectionType>;

      //! The type of WebSocketChannel used.
      using WebSocketChannel = typename HttpServer::WebSocketChannel;

      //! The type of HttpUpgradeSlot's used for WebSockets.
      using WebSocketSlot = typename HttpServer::WebSocketSlot;

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
          Details::GetSlots(*m_servlet),
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
