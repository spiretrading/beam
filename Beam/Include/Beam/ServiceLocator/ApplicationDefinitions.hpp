#ifndef BEAM_SERVICE_LOCATOR_APPLICATION_DEFINITIONS_HPP
#define BEAM_SERVICE_LOCATOR_APPLICATION_DEFINITIONS_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ProtocolServiceLocatorClient.hpp"
#include "Beam/Services/MessageProtocol.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/TimeService/LiveTimer.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/Streamable.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

namespace Beam {

  /** Stores the configuration needed to connect a ServiceLocatorClient. */
  struct ServiceLocatorClientConfig {

    /**
     * Creates a config by parsing a YAML node.
     * @param node The YAML node to parse.
     */
    static ServiceLocatorClientConfig parse(const YAML::Node& node);

    /** The address to connect to. */
    IpAddress m_address;

    /** The username to use. */
    std::string m_username;

    /** The password to login with. */
    std::string m_password;
  };

  /** Stores the configuration for a service. */
  struct ServiceConfiguration {

    /** The name of the service. */
    std::string m_name;

    /** The network interface to bind the service to. */
    IpAddress m_interface;

    /** The ServiceEntry properties to register. */
    JsonObject m_properties;

    /**
     * Parses a ServiceConfiguration from a YAML node.
     * @param node The YAML node containing the service configuration.
     * @param default_name The service's default name used if the <i>node</i>
     *        doesn't explicitly provide one.
     */
    static ServiceConfiguration parse(
      const YAML::Node& node, const std::string& default_name);
  };

  class ApplicationServiceLocatorClient : public ProtocolServiceLocatorClient<
      ServiceProtocolClientBuilder<
        MessageProtocol<std::unique_ptr<TcpSocketChannel>,
          BinarySender<SharedBuffer>>, LiveTimer>> {
    public:
      using ServiceProtocolClientBuilder =
        typename ProtocolServiceLocatorClient<
          Beam::ServiceProtocolClientBuilder<MessageProtocol<
            std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>>,
            LiveTimer>>::ServiceProtocolClientBuilder;

      /**
       * Constructs an ApplicationServiceLocatorClient.
       * @param username The username to login with.
       * @param password The password to login with.
       * @param address The address of the ServiceLocator.
       */
      ApplicationServiceLocatorClient(const std::string& username,
        const std::string& password, const IpAddress& address);

      /**
       * Constructs an ApplicationServiceLocatorClient from a configuration.
       * @param config The configuration to use.
       */
      explicit ApplicationServiceLocatorClient(
        ServiceLocatorClientConfig config);
  };

  /**
   * Registers a service with the ServiceLocator.
   * @param client The ServiceLocatorClient used to register the service.
   * @param config The ServiceConfiguration to register.
   */
  template<IsServiceLocatorClient C>
  ServiceEntry add(C& client, const ServiceConfiguration& config) {
    return try_or_nest([&] {
      return client.add(config.m_name, config.m_properties);
    }, std::runtime_error("Error registering service."));
  }

  inline ServiceLocatorClientConfig ServiceLocatorClientConfig::parse(
      const YAML::Node& node) {
    auto config = ServiceLocatorClientConfig();
    config.m_address = extract<IpAddress>(node, "address");
    config.m_username = extract<std::string>(node, "username");
    config.m_password = extract<std::string>(node, "password");
    return config;
  }

  inline ServiceConfiguration ServiceConfiguration::parse(
      const YAML::Node& node, const std::string& default_name) {
    auto config = ServiceConfiguration();
    config.m_name = extract<std::string>(node, "service", default_name);
    config.m_interface = extract<IpAddress>(node, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(config.m_interface);
    addresses = extract<std::vector<IpAddress>>(node, "addresses", addresses);
    config.m_properties["addresses"] =
      boost::lexical_cast<std::string>(Stream(addresses));
    return config;
  }

  inline ApplicationServiceLocatorClient::ApplicationServiceLocatorClient(
    const std::string& username, const std::string& password,
    const IpAddress& address)
    : ProtocolServiceLocatorClient<ServiceProtocolClientBuilder>(
        username, password, ServiceProtocolClientBuilder(
          [=] {
            return std::make_unique<TcpSocketChannel>(address);
          },
          [] {
            return std::make_unique<LiveTimer>(boost::posix_time::seconds(10));
          })) {}

  inline ApplicationServiceLocatorClient::ApplicationServiceLocatorClient(
    ServiceLocatorClientConfig config)
    : ApplicationServiceLocatorClient(std::move(config.m_username),
        std::move(config.m_password), std::move(config.m_address)) {}
}

#endif
