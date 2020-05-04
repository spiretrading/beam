#ifndef BEAM_SERVICEPROTOCOLCLIENTHANDLER_HPP
#define BEAM_SERVICEPROTOCOLCLIENTHANDLER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Services/Services.hpp"
#include "Beam/Services/ServiceSlots.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Utilities/ReportException.hpp"

namespace Beam {
namespace Services {

  /*! \class ServiceProtocolClientHandler
      \brief Instantiates ServiceProtocolClients for a service.
      \tparam ServiceProtocolClientBuilderType The type used to build
              ServiceProtocolClients.
   */
  template<typename ServiceProtocolClientBuilderType>
  class ServiceProtocolClientHandler : private boost::noncopyable {
    public:

      //! The type used to build ServiceProtocolClients.
      using ServiceProtocolClientBuilder = ServiceProtocolClientBuilderType;

      //! The type of ServiceProtocolClient used.
      using Client = typename ServiceProtocolClientHandler<
        ServiceProtocolClientBuilderType>::ServiceProtocolClientBuilder::Client;

      //! The type of function used to handle a reconnection event.
      /*!
        \param client The client used for the reconnection.
      */
      using ReconnectHandler =
        std::function<void(const std::shared_ptr<Client>&)>;

      //! Constructs a ServiceProtocolClientHandler.
      /*!
        \param builder Initializes the builder used for ServiceProtocolClients.
      */
      template<typename BuilderForward>
      ServiceProtocolClientHandler(BuilderForward&& builder);

      ~ServiceProtocolClientHandler();

      //! Returns the slots used by the ServiceProtocolClients.
      const ServiceSlots<Client>& GetSlots() const;

      //! Returns the slots used by the ServiceProtocolClients.
      ServiceSlots<Client>& GetSlots();

      //! Returns the most recently instantiated client.
      std::shared_ptr<Client> GetClient();

      //! Sets the function to call to handle a reconnection.
      /*!
        \param reconnectHandler The reconnection handler.
      */
      void SetReconnectHandler(const ReconnectHandler& reconnectHandler);

      void Open();

      void Close();

    private:
      Threading::Mutex m_mutex;
      typename OptionalLocalPtr<ServiceProtocolClientBuilderType>::type
        m_builder;
      ServiceSlots<Client> m_slots;
      ReconnectHandler m_reconnectHandler;
      std::shared_ptr<Client> m_client;
      Routines::RoutineHandlerGroup m_messageHandlers;
      IO::OpenState m_openState;

      void Shutdown();
      bool IsClientAvailable();
      void BuildClient();
      void MessageLoop(std::shared_ptr<Client> client);
  };

  template<typename ServiceProtocolClientBuilderType>
  template<typename BuilderForward>
  ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      ServiceProtocolClientHandler(BuilderForward&& builder)
      : m_builder(std::forward<BuilderForward>(builder)),
        m_reconnectHandler([] (const std::shared_ptr<Client>&) {}) {}

  template<typename ServiceProtocolClientBuilderType>
  ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      ~ServiceProtocolClientHandler() {
    Close();
  }

  template<typename ServiceProtocolClientBuilderType>
  const ServiceSlots<typename ServiceProtocolClientHandler<
      ServiceProtocolClientBuilderType>::Client>&
      ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      GetSlots() const {
    return m_slots;
  }

  template<typename ServiceProtocolClientBuilderType>
  ServiceSlots<typename ServiceProtocolClientHandler<
      ServiceProtocolClientBuilderType>::Client>&
      ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      GetSlots() {
    return m_slots;
  }

  template<typename ServiceProtocolClientBuilderType>
  std::shared_ptr<typename ServiceProtocolClientHandler<
      ServiceProtocolClientBuilderType>::Client>
      ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      GetClient() {
    std::shared_ptr<Client> client;
    {
      boost::lock_guard<Threading::Mutex> lock(m_mutex);
      if(m_client == nullptr) {
        BuildClient();
      }
      client = m_client;
    }
    return client;
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      SetReconnectHandler(const ReconnectHandler& reconnectHandler) {
    m_reconnectHandler = reconnectHandler;
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      BuildClient();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      Shutdown() {
    std::shared_ptr<Client> client;
    {
      boost::lock_guard<Threading::Mutex> lock(m_mutex);
      client = m_client;
    }
    if(client != nullptr) {
      client->Close();
    }
    m_messageHandlers.Wait();
    m_openState.SetClosed();
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      BuildClient() {
    std::shared_ptr<Client> client;
    while(true) {
      if(m_openState.IsClosing() || m_openState.IsClosed()) {
        BOOST_THROW_EXCEPTION(IO::NotConnectedException());
      }
      try {
        client = m_builder->Build(m_slots);
        m_builder->Open(*client);
        break;
      } catch(const IO::ConnectException&) {
        if(m_openState.IsOpen()) {
          Routines::Defer();
          continue;
        } else {
          BOOST_RETHROW;
        }
      } catch(const std::exception&) {
        if(m_openState.IsOpen()) {
          m_openState.SetClosed();
        }
        BOOST_RETHROW;
      }
      Routines::Defer();
    }
    m_client = client;
    if(m_openState.IsOpen()) {
      try {
        m_reconnectHandler(client);
      } catch(const std::exception&) {
        if(m_openState.IsOpen()) {
          m_openState.SetClosed();
        }
        return;
      }
    }
    m_messageHandlers.Spawn(
      std::bind(&ServiceProtocolClientHandler::MessageLoop, this, client));
  }

  template<typename ServiceProtocolClientBuilderType>
  void ServiceProtocolClientHandler<ServiceProtocolClientBuilderType>::
      MessageLoop(std::shared_ptr<Client> client) {
    try {
      while(true) {
        std::shared_ptr<Message<Client>> message = client->ReadMessage();
        BaseServiceSlot<Client>* slot = client->GetSlots().Find(*message);
        if(slot != nullptr) {
          message->EmitSignal(slot, Ref(*client));
        }
      }
    } catch(const std::exception&) {
      boost::lock_guard<Threading::Mutex> lock(m_mutex);
      try {
        BuildClient();
      } catch(const IO::NotConnectedException&) {
        return;
      }
    }
  };
}
}

#endif
