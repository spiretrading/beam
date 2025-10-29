#ifndef BEAM_HTTP_SERVLET_CONTAINER_HPP
#define BEAM_HTTP_SERVLET_CONTAINER_HPP
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/WebServices/HttpServer.hpp"

namespace Beam {
namespace Details {
  template<typename C, typename S>
  struct servlet_type {
    using type = typename S::template apply<C>::type;
  };

  template<typename C, typename S> requires IsConnection<S>
  struct servlet_type<C, S> {
    using type = S;
  };

  template<typename C, typename S>
  using servlet_type_t = typename servlet_type<C, S>::type;
}

  /**
   * Executes and manages an HttpServlet.
   * @tparam M The type of HttpServlet to host.
   * @tparam C The type of ServerConnection accepting Channels.
   * @tparam P The type of pointer to use with the Servlet.
   */
  template<typename M, typename C> requires IsServerConnection<dereference_t<C>>
  class HttpServletContainer {
    public:

      /** The type of HttpServlet to host. */
      using Servlet = Details::servlet_type_t<HttpServletContainer, M>;

      /** The type of ServerConnection accepting Channels. */
      using ServerConnection = dereference_t<C>;

      /** The type of HttpServer. */
      using HttpServer = Beam::HttpServer<C>;

      /** The type of WebSocketChannel used. */
      using WebSocketChannel = typename HttpServer::WebSocketChannel;

      /** The type of HttpUpgradeSlot's used for WebSockets. */
      using WebSocketSlot = typename HttpServer::WebSocketSlot;

      /**
       * Constructs the HttpServletContainer.
       * @param servlet Initializes the Servlet.
       * @param server_connection Accepts connections to the servlet.
       */
      template<Initializes<M> SF, Initializes<C> CF>
      HttpServletContainer(SF&& servlet, CF&& server_connection);

      ~HttpServletContainer();

      void close();

    private:
      local_ptr_t<Servlet> m_servlet;
      HttpServer m_server;

      HttpServletContainer(const HttpServletContainer&) = delete;
      HttpServletContainer& operator =(const HttpServletContainer&) = delete;
      std::vector<HttpRequestSlot> get_slots();
      std::vector<WebSocketSlot> get_web_socket_slots();
  };

  template<typename M, typename C> requires IsServerConnection<dereference_t<C>>
  template<Initializes<M> SF, Initializes<C> CF>
  HttpServletContainer<M, C>::HttpServletContainer(
    SF&& servlet, CF&& server_connection)
    : m_servlet(std::forward<SF>(servlet)),
      m_server(std::forward<CF>(server_connection), get_slots(),
        get_web_socket_slots()) {}

  template<typename M, typename C> requires IsServerConnection<dereference_t<C>>
  HttpServletContainer<M, C>::~HttpServletContainer() {
    close();
  }

  template<typename M, typename C> requires IsServerConnection<dereference_t<C>>
  void HttpServletContainer<M, C>::close() {
    m_server.close();
    m_servlet->close();
  }

  template<typename M, typename C> requires IsServerConnection<dereference_t<C>>
  std::vector<HttpRequestSlot> HttpServletContainer<M, C>::get_slots() {
    if constexpr(requires { m_servlet->get_slots(); }) {
      return m_servlet->get_slots();
    } else {
      return {};
    }
  }

  template<typename M, typename C> requires IsServerConnection<dereference_t<C>>
  std::vector<typename HttpServletContainer<M, C>::WebSocketSlot>
      HttpServletContainer<M, C>::get_web_socket_slots() {
    if constexpr(requires { m_servlet->get_web_socket_slots(); }) {
      return m_servlet->get_web_socket_slots();
    } else {
      return {};
    }
  }
}

#endif
