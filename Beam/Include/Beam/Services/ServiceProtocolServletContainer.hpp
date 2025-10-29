#ifndef BEAM_SERVICE_PROTOCOL_SERVLET_CONTAINER_HPP
#define BEAM_SERVICE_PROTOCOL_SERVLET_CONTAINER_HPP
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Services/ServiceProtocolServer.hpp"
#include "Beam/Services/ServiceProtocolServlet.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {

  /**
   * Executes and manages a ServiceProtocolServlet.
   * @tparam M The type of ServiceProtocolServlet to host.
   * @tparam C The type of ServerConnection accepting Channels.
   * @tparam S The type of Sender used for serialization.
   * @tparam E The type of Encoder used for messages.
   * @tparam T The type of Timer used for heartbeats.
   * @tparam P The type of pointer to use with the Servlet.
   */
  template<typename M, typename C, typename S, typename E, typename T,
    typename P = LocalPointerPolicy> requires
      IsServerConnection<dereference_t<C>> && IsSender<S> &&
        IsEncoder<E> && IsTimer<dereference_t<T>>
  class ServiceProtocolServletContainer {
    public:

      /** The type of ServiceProtocolServlet to host. */
      using Servlet = typename M::template apply<
        ServiceProtocolServletContainer>::type;

      /** The type of ServerConnection accepting Channels. */
      using ServerConnection = dereference_t<C>;

      /** The type of Sender used for serialization. */
      using Sender = S;

      /** The type of Encoder used for messages. */
      using Encoder = E;

      /** The type of Timer used for heartbeats. */
      using Timer = dereference_t<T>;

      /** The type of ServiceProtocolServer. */
      using ServiceProtocolServer = Beam::ServiceProtocolServer<
        C, Sender, Encoder, T, typename M::Session, supports_parallelism_v<M>>;

      /** The type of ServiceProtocolClient. */
      using ServiceProtocolClient =
        typename ServiceProtocolServer::ServiceProtocolClient;

      /**
       * Constructs the ServiceProtocolServletContainer.
       * @param servlet Initializes the Servlet.
       * @param server_connection Accepts connections to the servlet.
       * @param timer_factory The type of Timer used for heartbeats.
       */
      template<typename SF, typename CF>
      ServiceProtocolServletContainer(SF&& servlet, CF&& server_connection,
        typename ServiceProtocolServer::TimerFactory timer_factory);

      ~ServiceProtocolServletContainer();

      void close();

    private:
      typename P::template apply<Servlet>::type m_servlet;
      Async<void> m_is_open;
      ServiceProtocolServer m_protocol_server;

      ServiceProtocolServletContainer(
        const ServiceProtocolServletContainer&) = delete;
      ServiceProtocolServletContainer& operator =(
        const ServiceProtocolServletContainer&) = delete;
      void on_accept(ServiceProtocolClient& client);
      void on_close(ServiceProtocolClient& client);
  };

  template<typename M, typename C, typename S, typename E, typename T,
    typename P> requires IsServerConnection<dereference_t<C>> && IsSender<S> &&
      IsEncoder<E> && IsTimer<dereference_t<T>>
  template<typename SF, typename CF>
  ServiceProtocolServletContainer<M, C, S, E, T, P>::
      ServiceProtocolServletContainer(SF&& servlet, CF&& server_connection,
      typename ServiceProtocolServer::TimerFactory timer_factory)
BEAM_SUPPRESS_THIS_INITIALIZER()
      try : m_servlet(std::forward<SF>(servlet)),
            m_protocol_server(std::forward<CF>(server_connection),
              std::move(timer_factory), std::bind_front(
                &ServiceProtocolServletContainer::on_accept, this),
              std::bind_front(
                &ServiceProtocolServletContainer::on_close, this)) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    m_servlet->register_services(out(m_protocol_server.get_slots()));
    m_is_open.get_eval().set();
  } catch(const std::exception&) {
    std::throw_with_nested(ConnectException("Failed to open server."));
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P> requires IsServerConnection<dereference_t<C>> && IsSender<S> &&
      IsEncoder<E> && IsTimer<dereference_t<T>>
  ServiceProtocolServletContainer<M, C, S, E, T, P>::
      ~ServiceProtocolServletContainer() {
    close();
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P> requires IsServerConnection<dereference_t<C>> && IsSender<S> &&
      IsEncoder<E> && IsTimer<dereference_t<T>>
  void ServiceProtocolServletContainer<M, C, S, E, T, P>::close() {
    m_protocol_server.close();
    m_servlet->close();
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P> requires IsServerConnection<dereference_t<C>> && IsSender<S> &&
      IsEncoder<E> && IsTimer<dereference_t<T>>
  void ServiceProtocolServletContainer<M, C, S, E, T, P>::on_accept(
      ServiceProtocolClient& client) {
    m_is_open.get();
    if constexpr(requires { m_servlet->handle_accept(client); }) {
      m_servlet->handle_accept(client);
    }
  }

  template<typename M, typename C, typename S, typename E, typename T,
    typename P> requires IsServerConnection<dereference_t<C>> && IsSender<S> &&
      IsEncoder<E> && IsTimer<dereference_t<T>>
  void ServiceProtocolServletContainer<M, C, S, E, T, P>::on_close(
      ServiceProtocolClient& client) {
    if constexpr(requires { m_servlet->handle_close(client); }) {
      m_servlet->handle_close(client);
    }
  }
}

#endif
