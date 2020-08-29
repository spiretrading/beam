#ifndef BEAM_SERVICE_LOCATOR_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICE_LOCATOR_APPLICATION_DEFINITIONS_HPP
#include <optional>
#include <string>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include <boost/noncopyable.hpp>

namespace Beam::ServiceLocator {

  /** Stores the configuration needed to connect a ServiceLocatorClient. */
  struct ServiceLocatorClientConfig {

    /**
     * Creates a config by parsing a YAML node.
     * @param node The YAML node to parse.
     */
    static ServiceLocatorClientConfig Parse(const YAML::Node& node);

    /** The address to connect to. */
    Network::IpAddress m_address;

    /** The username to use. */
    std::string m_username;

    /** The password to login with. */
    std::string m_password;
  };

  /** Encapsulates a standard ServiceLocatorClient used in an application. */
  class ApplicationServiceLocatorClient : private boost::noncopyable {
    public:

      /** The type of session builder used by the client. */
      using SessionBuilder = Services::ServiceProtocolClientBuilder<
        Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
        Serialization::BinarySender<IO::SharedBuffer>>, Threading::LiveTimer>;

      /** Defines the standard ServiceLocatorClient used for applications. */
      using Client = ServiceLocatorClient<SessionBuilder>;

      /** Constructs an ApplicationServiceLocatorClient. */
      ApplicationServiceLocatorClient() = default;

      /**
       * Builds the session.
       * @param username The session's username.
       * @param password The session's password.
       * @param address The IP address to connect to.
       * @param socketThreadPool The SocketThreadPool used for the socket
       *        connection.
       * @param timerThreadPool The TimerThreadPool used for heartbeats.
       */
      void BuildSession(std::string username, std::string password,
        const Network::IpAddress& address,
        Ref<Network::SocketThreadPool> socketThreadPool,
        Ref<Threading::TimerThreadPool> timerThreadPool);

      /** Returns a reference to the Client. */
      Client& operator *();

      /** Returns a reference to the Client. */
      const Client& operator *() const;

      /** Returns a pointer to the Client. */
      Client* operator ->();

      /** Returns a pointer to the Client. */
      const Client* operator ->() const;

      /** Returns a pointer to the Client. */
      Client* Get();

      /** Returns a pointer to the Client. */
      const Client* Get() const;

    private:
      std::optional<Client> m_client;
  };

  inline ServiceLocatorClientConfig ServiceLocatorClientConfig::Parse(
      const YAML::Node& node) {
    auto config = ServiceLocatorClientConfig();
    config.m_address = Extract<Network::IpAddress>(node, "address");
    config.m_username = Extract<std::string>(node, "username");
    config.m_password = Extract<std::string>(node, "password");
    return config;
  }

  inline void ApplicationServiceLocatorClient::BuildSession(
      std::string username, std::string password,
      const Network::IpAddress& address,
      Ref<Network::SocketThreadPool> socketThreadPool,
      Ref<Threading::TimerThreadPool> timerThreadPool) {
    if(m_client.has_value()) {
      m_client->Close();
      m_client = std::nullopt;
    }
    auto socketThreadPoolHandle = socketThreadPool.Get();
    auto timerThreadPoolHandle = timerThreadPool.Get();
    auto isConnected = false;
    auto sessionBuilder = SessionBuilder(
      [=] () mutable {
        if(isConnected) {
          BOOST_THROW_EXCEPTION(IO::NotConnectedException());
        }
        isConnected = true;
        return std::make_unique<Network::TcpSocketChannel>(address,
          Ref(*socketThreadPoolHandle));
      },
      [=] {
        return std::make_unique<Threading::LiveTimer>(
          boost::posix_time::seconds(10), Ref(*timerThreadPoolHandle));
      });
    m_client.emplace(std::move(username), std::move(password), sessionBuilder);
  }

  inline ApplicationServiceLocatorClient::Client&
      ApplicationServiceLocatorClient::operator *() {
    return *m_client;
  }

  inline const ApplicationServiceLocatorClient::Client&
      ApplicationServiceLocatorClient::operator *() const {
    return *m_client;
  }

  inline ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::operator ->() {
    return &*m_client;
  }

  inline const ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::operator ->() const {
    return &*m_client;
  }

  inline ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::Get() {
    return &*m_client;
  }

  inline const ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::Get() const {
    return &*m_client;
  }
}

#endif
