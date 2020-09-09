#ifndef BEAM_AUTHENTICATION_SERVLET_ADAPTER_HPP
#define BEAM_AUTHENTICATION_SERVLET_ADAPTER_HPP
#include <type_traits>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam::ServiceLocator {

  /**
   * Augments a Servlet to support authenticating with a ServiceLocator.
   * @param <C> The container instantiating this servlet.
   * @param <S> The Servler to augment.
   * @param <L> The type of ServiceLocatorClient connected to the
   *            ServiceLocator.
   */
  template<typename C, typename S, typename L>
  class AuthenticationServletAdapter {
    public:
      using Container = C;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;
      using Servlet = GetTryDereferenceType<S>;

      /**
       * Constructs an AuthenticationServletAdapter.
       * @param serviceLocatorClient Used to initialize the
       *        ServiceLocatorClient.
       * @param servlet Used to initialize the Servlet.
       */
      template<typename LF, typename SF>
      AuthenticationServletAdapter(LF&& serviceLocatorClient, SF&& servlet);

      ~AuthenticationServletAdapter();

      void RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
        slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Close();

    private:
      GetOptionalLocalPtr<L> m_serviceLocatorClient;
      GetOptionalLocalPtr<S> m_servlet;
      IO::OpenState m_openState;

      AuthenticationServletAdapter(
        const AuthenticationServletAdapter&) = delete;
      AuthenticationServletAdapter& operator =(
        const AuthenticationServletAdapter&) = delete;
      void OnSendSessionIdRequest(
        Services::RequestToken<ServiceProtocolClient, SendSessionIdService>&
        request, unsigned int key, const std::string& sessionId);
      void OnServiceRequest(ServiceProtocolClient& client);
  };

  template<typename BaseSession, typename = void>
  class AuthenticationServletSession : public AuthenticatedSession,
    public BaseSession {};

  template<typename BaseSession>
  class AuthenticationServletSession<BaseSession, std::enable_if_t<
    std::is_base_of_v<AuthenticatedSession, BaseSession>>> :
    public BaseSession {};

  template<typename S, typename L, typename P = LocalPointerPolicy>
  struct MetaAuthenticationServletAdapter {
    static constexpr auto SupportsParallelism =
      Services::SupportsParallelism<S>::value;
    using Session = AuthenticationServletSession<typename S::Session>;
    template<typename C>
    struct apply {
      using type = AuthenticationServletAdapter<C, typename P::template apply<
        typename S::template apply<C>::type>::type, L>;
    };
  };

  template<typename C, typename S, typename L>
  template<typename LF, typename SF>
  AuthenticationServletAdapter<C, S, L>::AuthenticationServletAdapter(
    LF&& serviceLocatorClient, SF&& servlet)
    : m_serviceLocatorClient(std::forward<LF>(serviceLocatorClient)),
      m_servlet(std::forward<SF>(servlet)) {}

  template<typename C, typename S, typename L>
  AuthenticationServletAdapter<C, S, L>::~AuthenticationServletAdapter() {
    Close();
  }

  template<typename C, typename S, typename L>
  void AuthenticationServletAdapter<C, S, L>::RegisterServices(
      Out<Services::ServiceSlots<ServiceProtocolClient>> slots) {
    slots->GetRegistry().template Register<SendSessionIdService::Request<
      ServiceProtocolClient>>(
      "Beam.ServiceLocator.SendSessionIdService.Request");
    slots->GetRegistry().template Register<SendSessionIdService::Response<
      ServiceProtocolClient>>(
      "Beam.ServiceLocator.SendSessionIdService.Response");
    SendSessionIdService::AddRequestSlot(Store(slots), std::bind(
      &AuthenticationServletAdapter::OnSendSessionIdRequest, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Services::ServiceSlots<ServiceProtocolClient> servletSlots;
    m_servlet->RegisterServices(Store(servletSlots));
    auto serviceRequestPreHook = std::bind(
      &AuthenticationServletAdapter::OnServiceRequest, this,
      std::placeholders::_1);
    servletSlots.Apply([&] (auto& name, auto& slot) {
      slot.AddPreHook(serviceRequestPreHook);
    });
    slots->Add(std::move(servletSlots));
  }

  template<typename C, typename S, typename L>
  void AuthenticationServletAdapter<C, S, L>::HandleClientAccepted(
    ServiceProtocolClient& client) {}

  template<typename C, typename S, typename L>
  void AuthenticationServletAdapter<C, S, L>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Services::Details::InvokeClientClosed<
      Services::Details::HasClientClosedMethod<Servlet,
      ServiceProtocolClient>::value>()(*m_servlet, client);
  }

  template<typename C, typename S, typename L>
  void AuthenticationServletAdapter<C, S, L>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_servlet->Close();
    m_openState.Close();
  }

  template<typename C, typename S, typename L>
  void AuthenticationServletAdapter<C, S, L>::OnSendSessionIdRequest(
      Services::RequestToken<ServiceProtocolClient, SendSessionIdService>&
      request, unsigned int key, const std::string& sessionId) {
    auto& session = request.GetSession();
    try {
      auto account = m_serviceLocatorClient->AuthenticateSession(sessionId,
        key);
      session.SetAccount(account);
      request.SetResult();
      Services::Details::InvokeClientAccepted<
        Services::Details::HasClientAcceptedMethod<Servlet,
        ServiceProtocolClient>::value>()(*m_servlet, request.GetClient());
    } catch(const std::exception& e) {
      request.SetException(Services::ServiceRequestException(e.what()));
    }
  }

  template<typename C, typename S, typename L>
  void AuthenticationServletAdapter<C, S, L>::OnServiceRequest(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException("Not logged in.");
    }
  }
}

#endif
