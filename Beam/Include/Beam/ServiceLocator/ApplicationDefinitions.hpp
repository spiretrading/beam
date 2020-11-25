#ifndef BEAM_SERVICE_LOCATOR_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICE_LOCATOR_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/Streamable.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

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
  class ApplicationServiceLocatorClient {
    public:

      /** The type used to build client sessions. */
      using SessionBuilder = Services::ServiceProtocolClientBuilder<
        Services::MessageProtocol<std::unique_ptr<Network::TcpSocketChannel>,
        Serialization::BinarySender<IO::SharedBuffer>>, Threading::LiveTimer>;

      /** Defines the standard ServiceLocatorClient used for applications. */
      using Client = ServiceLocatorClient<SessionBuilder>;

      /**
       * Constructs an ApplicationServiceLocatorClient.
       * @param username The session's username.
       * @param password The session's password.
       * @param address The IP address to connect to.
       */
      ApplicationServiceLocatorClient(std::string username,
        std::string password, Network::IpAddress address);

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
      Client m_client;

      ApplicationServiceLocatorClient(
        const ApplicationServiceLocatorClient&) = delete;
      ApplicationServiceLocatorClient& operator =(
        const ApplicationServiceLocatorClient&) = delete;
  };

  /** Stores the configuration for a service. */
  struct ServiceConfiguration {

    /** The name of the service. */
    std::string m_name;

    /** The network interface to bind the service to. */
    Network::IpAddress m_interface;

    /** The ServiceEntry properties to register. */
    JsonObject m_properties;

    /**
     * Parses a ServiceConfiguration from a YAML node.
     * @param node The YAML node containing the service configuration.
     * @param defaultName The service's default name used if the <i>node</i>
     *        doesn't explicitly provide one.
     */
    static ServiceConfiguration Parse(const YAML::Node& node,
      const std::string& defaultName);
  };

  /**
   * Registers a service with the ServiceLocator.
   * @param client The ServiceLocatorClient used to register the service.
   * @param config The ServiceConfiguration to register.
   */
  template<typename ServiceLocatorClient>
  ServiceEntry Register(ServiceLocatorClient& client,
      const ServiceConfiguration& config) {
    return TryOrNest([&] {
      return client.Register(config.m_name, config.m_properties);
    }, std::runtime_error("Error registering service."));
  }

  /**
   * Builds an ApplicationServiceLocatorClient whose connection details are
   * parsed from a YAML node.
   * @param node The YAML node to parse.
   * @return An ApplicationServiceLocatorClient connected to the service locator
   *         specified by the <i>node</i>.
   */
  inline ApplicationServiceLocatorClient MakeApplicationServiceLocatorClient(
      const YAML::Node& node) {
    auto config = TryOrNest([&] {
      return ServiceLocatorClientConfig::Parse(node);
    }, std::runtime_error("Error parsing service locator config."));
    return TryOrNest([&] {
      return ApplicationServiceLocatorClient(config.m_username,
        config.m_password, config.m_address);
    }, std::runtime_error("Error logging in."));
  }

  inline ServiceLocatorClientConfig ServiceLocatorClientConfig::Parse(
      const YAML::Node& node) {
    auto config = ServiceLocatorClientConfig();
    config.m_address = Extract<Network::IpAddress>(node, "address");
    config.m_username = Extract<std::string>(node, "username");
    config.m_password = Extract<std::string>(node, "password");
    return config;
  }

  inline ServiceConfiguration ServiceConfiguration::Parse(
      const YAML::Node& node, const std::string& defaultName) {
    auto config = ServiceConfiguration();
    config.m_name = Extract<std::string>(node, "service", defaultName);
    config.m_interface = Extract<Network::IpAddress>(node, "interface");
    auto addresses = std::vector<Network::IpAddress>();
    addresses.push_back(config.m_interface);
    addresses = Extract<std::vector<Network::IpAddress>>(node, "addresses",
      addresses);
    config.m_properties["addresses"] = boost::lexical_cast<std::string>(
      Stream(addresses));
    return config;
  }

  inline ApplicationServiceLocatorClient::ApplicationServiceLocatorClient(
    std::string username, std::string password, Network::IpAddress address)
    : m_client(std::move(username), std::move(password),
        SessionBuilder(
          [=] {
            return std::make_unique<Network::TcpSocketChannel>(address);
          },
          [] {
            return std::make_unique<Threading::LiveTimer>(
              boost::posix_time::seconds(10));
          })) {}

  inline ApplicationServiceLocatorClient::Client&
      ApplicationServiceLocatorClient::operator *() {
    return m_client;
  }

  inline const ApplicationServiceLocatorClient::Client&
      ApplicationServiceLocatorClient::operator *() const {
    return m_client;
  }

  inline ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::operator ->() {
    return &m_client;
  }

  inline const ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::operator ->() const {
    return &m_client;
  }

  inline ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::Get() {
    return &m_client;
  }

  inline const ApplicationServiceLocatorClient::Client*
      ApplicationServiceLocatorClient::Get() const {
    return &m_client;
  }
}

#endif
