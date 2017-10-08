#ifndef BEAM_AUTHENTICATIONSERVLETADAPTER_HPP
#define BEAM_AUTHENTICATIONSERVLETADAPTER_HPP
#include <type_traits>
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/LocalPointerPolicy.hpp"
#include "Beam/ServiceLocator/AuthenticatedSession.hpp"
#include "Beam/ServiceLocator/ServiceLocatorServices.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Services/ServiceRequestException.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class AuthenticationServletAdapter
      \brief Augments a Servlet to support authenticating with a ServiceLocator.
      \tparam ContainerType The container instantiating this servlet.
      \tparam ServletType The Servler to augment.
      \tparam ServiceLocatorClientType The type of ServiceLocatorClient
                                       connected to the ServiceLocator.
   */
  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  class AuthenticationServletAdapter : private boost::noncopyable {
    public:
      using Container = ContainerType;
      using ServiceProtocolClient = typename Container::ServiceProtocolClient;
      using Servlet = GetTryDereferenceType<ServletType>;

      //! Constructs an AuthenticationServletAdapter.
      /*!
        \param serviceLocatorClient Used to initialize the ServiceLocatorClient.
        \param servlet Used to initialize the Servlet.
      */
      template<typename ServiceLocatorClientForward, typename ServletForward>
      AuthenticationServletAdapter(ServiceLocatorClientForward&&
        serviceLocatorClient, ServletForward&& servlet);

      ~AuthenticationServletAdapter();

      void RegisterServices(Out<Services::ServiceSlots<ServiceProtocolClient>>
        slots);

      void HandleClientAccepted(ServiceProtocolClient& client);

      void HandleClientClosed(ServiceProtocolClient& client);

      void Open();

      void Close();

    private:
      GetOptionalLocalPtr<ServiceLocatorClientType> m_serviceLocatorClient;
      GetOptionalLocalPtr<ServletType> m_servlet;
      IO::OpenState m_openState;

      void Shutdown();
      void OnSendSessionIdRequest(
        Services::RequestToken<ServiceProtocolClient, SendSessionIdService>&
        request, unsigned int key, const std::string& sessionId);
      void OnServiceRequest(ServiceProtocolClient& client);
  };

  template<typename BaseSession, typename = void>
  class AuthenticationServletSession : public AuthenticatedSession,
    public BaseSession {};

  template<typename BaseSession>
  class AuthenticationServletSession<BaseSession,
    std::enable_if_t<std::is_base_of<
    AuthenticatedSession, BaseSession>::value>> : public BaseSession {};

  template<typename MetaServlet, typename ServiceLocatorClientType,
    typename ServletPointerPolicy = LocalPointerPolicy>
  struct MetaAuthenticationServletAdapter {
    static constexpr bool SupportsParallelism =
      Services::SupportsParallelism<MetaServlet>::value;
    using Session = AuthenticationServletSession<typename MetaServlet::Session>;
    template<typename ContainerType>
    struct apply {
      using type = AuthenticationServletAdapter<ContainerType,
        typename ServletPointerPolicy::template apply<
        typename MetaServlet::template apply<ContainerType>::type>::type,
        ServiceLocatorClientType>;
    };
  };

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  template<typename ServiceLocatorClientForward, typename ServletForward>
  AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::AuthenticationServletAdapter(
      ServiceLocatorClientForward&& serviceLocatorClient,
      ServletForward&& servlet)
      : m_serviceLocatorClient{std::forward<ServiceLocatorClientForward>(
          serviceLocatorClient)},
        m_servlet{std::forward<ServletForward>(servlet)} {}

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::~AuthenticationServletAdapter() {
    Close();
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::RegisterServices(
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
    servletSlots.Apply(
      [&] (auto& name, auto& slot) {
        slot.AddPreHook(serviceRequestPreHook);
      });
    slots->Acquire(std::move(servletSlots));
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::HandleClientAccepted(
      ServiceProtocolClient& client) {}

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::HandleClientClosed(
      ServiceProtocolClient& client) {
    Services::Details::InvokeClientClosed<
      Services::Details::HasClientClosedMethod<Servlet,
      ServiceProtocolClient>::value>()(*m_servlet, client);
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_servlet->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::Shutdown() {
    m_servlet->Close();
    m_openState.SetClosed();
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::OnSendSessionIdRequest(
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
      request.SetException(Services::ServiceRequestException{e.what()});
    }
  }

  template<typename ContainerType, typename ServletType,
    typename ServiceLocatorClientType>
  void AuthenticationServletAdapter<ContainerType, ServletType,
      ServiceLocatorClientType>::OnServiceRequest(
      ServiceProtocolClient& client) {
    auto& session = client.GetSession();
    if(!session.IsLoggedIn()) {
      throw Services::ServiceRequestException{"Not logged in."};
    }
  }
}
}

#endif
