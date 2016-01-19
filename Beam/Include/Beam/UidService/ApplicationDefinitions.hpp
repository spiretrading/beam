#ifndef BEAM_UIDAPPLICATIONDEFINITIONS_HPP
#define BEAM_UIDAPPLICATIONDEFINITIONS_HPP
#include <string>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SizeDeclarativeReader.hpp"
#include "Beam/IO/SizeDeclarativeWriter.hpp"
#include "Beam/IO/WrapperChannel.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/Services/AuthenticatedServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidService.hpp"
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>

namespace Beam {
namespace UidService {
namespace Details {
  using UidClientSessionBuilder =
    Services::AuthenticatedServiceProtocolClientBuilder<
    ServiceLocator::ApplicationServiceLocatorClient::Client,
    Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
    Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
    Threading::LiveTimer>;
}

  /*! \class ApplicationUidClient
      \brief Encapsulates a standard UidClient used in an application.
   */
  class ApplicationUidClient : private boost::noncopyable {
    public:

      //! Defines the standard UidClient used for applications.
      using Client = UidClient<Details::UidClientSessionBuilder>;

      //! Constructs an ApplicationUidClient.
      ApplicationUidClient() = default;

      //! Builds the session.
      /*!
        \param serviceLocatorClient The ServiceLocatorClient used to
               authenticate sessions.
        \param socketThreadPool The SocketThreadPool used for the socket
               connection.
        \param timerThreadPool The TimerThreadPool used for heartbeats.
      */
      void BuildSession(RefType<ServiceLocator::
        ApplicationServiceLocatorClient::Client> serviceLocatorClient,
        RefType<Network::SocketThreadPool> socketThreadPool,
        RefType<Threading::TimerThreadPool> timerThreadPool);

      //! Returns a reference to the Client.
      Client& operator *();

      //! Returns a reference to the Client.
      const Client& operator *() const;

      //! Returns a pointer to the Client.
      Client* operator ->();

      //! Returns a pointer to the Client.
      const Client* operator ->() const;

      //! Returns a pointer to the Client.
      Client* Get();

      //! Returns a pointer to the Client.
      const Client* Get() const;

    private:
      DelayPtr<Client> m_client;
  };

  inline void ApplicationUidClient::BuildSession(
      RefType<ServiceLocator::ApplicationServiceLocatorClient::Client>
      serviceLocatorClient, RefType<Network::SocketThreadPool> socketThreadPool,
      RefType<Threading::TimerThreadPool> timerThreadPool) {
    if(m_client.IsInitialized()) {
      m_client->Close();
      m_client.Reset();
    }
    auto serviceLocatorClientHandle = serviceLocatorClient.Get();
    auto socketThreadPoolHandle = socketThreadPool.Get();
    auto timerThreadPoolHandle = timerThreadPool.Get();
    auto addresses = ServiceLocator::LocateServiceAddresses(
      *serviceLocatorClientHandle, SERVICE_NAME);
    auto delay = false;
    Details::UidClientSessionBuilder sessionBuilder(Ref(serviceLocatorClient),
      [=] () mutable {
        if(delay) {
          Threading::LiveTimer delayTimer(boost::posix_time::seconds(3),
            Ref(*timerThreadPoolHandle));
          delayTimer.Start();
          delayTimer.Wait();
        }
        delay = true;
        return std::make_unique<Network::TcpSocketChannel>(addresses,
          Ref(*socketThreadPoolHandle));
      },
      [=] {
        return std::make_unique<Threading::LiveTimer>(
          boost::posix_time::seconds(10), Ref(*timerThreadPoolHandle));
      });
    m_client.Initialize(sessionBuilder);
  }

  inline ApplicationUidClient::Client& ApplicationUidClient::operator *() {
    return m_client.Get();
  }

  inline const ApplicationUidClient::Client&
      ApplicationUidClient::operator *() const {
    return m_client.Get();
  }

  inline ApplicationUidClient::Client* ApplicationUidClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationUidClient::Client*
      ApplicationUidClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationUidClient::Client* ApplicationUidClient::Get() {
    return &*m_client;
  }

  inline const ApplicationUidClient::Client* ApplicationUidClient::Get() const {
    return &*m_client;
  }
}
}

#endif
