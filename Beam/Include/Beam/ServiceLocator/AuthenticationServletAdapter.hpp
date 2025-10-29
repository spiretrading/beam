#ifndef BEAM_AUTHENTICATION_SERVLET_ADAPTER_HPP
#define BEAM_AUTHENTICATION_SERVLET_ADAPTER_HPP
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceRequestException.hpp"
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Augments a Servlet to support authenticating with a ServiceLocator.
   * @tparam C The container instantiating this servlet.
   * @tparam S The Servlet to augment.
   * @tparam L The type of ServiceLocatorClient connected to the
   *            ServiceLocator.
   */
  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  class AuthenticationServletAdapter {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;
      using Servlet = dereference_t<S>;

      /**
       * Constructs an AuthenticationServletAdapter.
       * @param client Used to initialize the ServiceLocatorClient.
       * @param servlet Used to initialize the Servlet.
       */
      template<Initializes<L> LF, Initializes<S> SF>
      AuthenticationServletAdapter(LF&& client, SF&& servlet);

      ~AuthenticationServletAdapter();

      void register_services(Out<ServiceSlots<ServiceProtocolClient>> slots);
      void handle_accept(typename Container::ServiceProtocolClient& client);
      void handle_close(typename Container::ServiceProtocolClient& client);
      void close();

    private:
      local_ptr_t<L> m_client;
      local_ptr_t<S> m_servlet;
      OpenState m_open_state;

      AuthenticationServletAdapter(
        const AuthenticationServletAdapter&) = delete;
      AuthenticationServletAdapter& operator =(
        const AuthenticationServletAdapter&) = delete;
      void on_send_session_id_request(RequestToken<ServiceProtocolClient,
        ServiceLocatorServices::SendSessionIdService>& request,
        unsigned int key, const std::string& session_id);
      void on_service_request(ServiceProtocolClient& client);
  };

  template<typename BaseSession>
  class AuthenticationServletSession :
    public AuthenticatedSession, public BaseSession {};

  template<typename BaseSession> requires
    std::is_base_of_v<AuthenticatedSession, BaseSession>
  class AuthenticationServletSession<BaseSession> : public BaseSession {};

  template<typename S, typename L, typename P = LocalPointerPolicy>
  struct MetaAuthenticationServletAdapter {
    using Session = AuthenticationServletSession<typename S::Session>;
    static constexpr auto SUPPORTS_PARALLELISM = supports_parallelism_v<S>;
    template<typename C>
    struct apply {
      using type = AuthenticationServletAdapter<C, typename P::template apply<
        typename S::template apply<C>::type>::type, L>;
    };
  };

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  template<Initializes<L> LF, Initializes<S> SF>
  AuthenticationServletAdapter<C, S, L>::AuthenticationServletAdapter(
    LF&& client, SF&& servlet)
    : m_client(std::forward<LF>(client)),
      m_servlet(std::forward<SF>(servlet)) {}

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  AuthenticationServletAdapter<C, S, L>::~AuthenticationServletAdapter() {
    close();
  }

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  void AuthenticationServletAdapter<C, S, L>::register_services(
      Out<ServiceSlots<ServiceProtocolClient>> slots) {
    slots->get_registry().template add<
      ServiceLocatorServices::SendSessionIdService::Request<
        ServiceProtocolClient>>(
          "Beam.ServiceLocator.SendSessionIdService.Request");
    slots->get_registry().template add<
      ServiceLocatorServices::SendSessionIdService::Response<
        ServiceProtocolClient>>(
          "Beam.ServiceLocator.SendSessionIdService.Response");
    ServiceLocatorServices::SendSessionIdService::add_request_slot(
      out(slots), std::bind_front(
        &AuthenticationServletAdapter::on_send_session_id_request, this));
    auto servlet_slots = ServiceSlots<ServiceProtocolClient>();
    m_servlet->register_services(out(servlet_slots));
    auto service_request_pre_hook =
      std::bind_front(&AuthenticationServletAdapter::on_service_request, this);
    servlet_slots.apply([&] (const auto& name, auto& slot) {
      slot.add_pre_hook(service_request_pre_hook);
    });
    slots->add(std::move(servlet_slots));
  }

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  void AuthenticationServletAdapter<C, S, L>::handle_accept(
    ServiceProtocolClient& client) {}

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  void AuthenticationServletAdapter<C, S, L>::handle_close(
      ServiceProtocolClient& client) {
    if constexpr(requires { m_servlet->handle_close(client); }) {
      m_servlet->handle_close(client);
    }
  }

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  void AuthenticationServletAdapter<C, S, L>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_servlet->close();
    m_open_state.close();
  }

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  void AuthenticationServletAdapter<C, S, L>::on_send_session_id_request(
      RequestToken<ServiceProtocolClient,
        ServiceLocatorServices::SendSessionIdService>&
          request, unsigned int key, const std::string& session_id) {
    auto& session = request.get_session();
    try {
      auto account = m_client->authenticate_session(session_id, key);
      session.set_account(account);
      request.set();
      if constexpr(
          requires { m_servlet->handle_accept(request.get_client()); }) {
        m_servlet->handle_accept(request.get_client());
      }
    } catch(const std::exception& e) {
      request.set_exception(ServiceRequestException(e.what()));
    }
  }

  template<typename C, typename S, typename L> requires
    IsServiceLocatorClient<dereference_t<L>>
  void AuthenticationServletAdapter<C, S, L>::on_service_request(
      ServiceProtocolClient& client) {
    auto& session = client.get_session();
    if(!session.is_logged_in()) {
      boost::throw_with_location(ServiceRequestException("Not logged in."));
    }
  }
}

#endif
