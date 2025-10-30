#include <doctest/doctest.h>
#include <yaml-cpp/yaml.h>
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"

using namespace Beam;
using namespace boost;

TEST_SUITE("ServiceLocatorClientConfig") {
  TEST_CASE("parse_complete_config") {
    auto yaml = YAML::Load(R"(
      address: "192.168.1.100:8080"
      username: test_user
      password: test_password
    )");
    auto config = ServiceLocatorClientConfig::parse(yaml);
    REQUIRE(config.m_address.get_host() == "192.168.1.100");
    REQUIRE(config.m_address.get_port() == 8080);
    REQUIRE(config.m_username == "test_user");
    REQUIRE(config.m_password == "test_password");
  }

  TEST_CASE("parse_localhost") {
    auto yaml = YAML::Load(R"(
      address: "localhost:9000"
      username: admin
      password: admin123
    )");
    auto config = ServiceLocatorClientConfig::parse(yaml);
    REQUIRE(config.m_address.get_host() == "localhost");
    REQUIRE(config.m_address.get_port() == 9000);
    REQUIRE(config.m_username == "admin");
    REQUIRE(config.m_password == "admin123");
  }

  TEST_CASE("parse_empty_username") {
    auto yaml = YAML::Load(R"(
      address: "10.0.0.1:5000"
      username: ""
      password: pass
    )");
    auto config = ServiceLocatorClientConfig::parse(yaml);
    REQUIRE(config.m_address.get_host() == "10.0.0.1");
    REQUIRE(config.m_address.get_port() == 5000);
    REQUIRE(config.m_username.empty());
    REQUIRE(config.m_password == "pass");
  }

  TEST_CASE("parse_empty_password") {
    auto yaml = YAML::Load(R"(
      address: "172.16.0.1:7000"
      username: user
      password: ""
    )");
    auto config = ServiceLocatorClientConfig::parse(yaml);
    REQUIRE(config.m_address.get_host() == "172.16.0.1");
    REQUIRE(config.m_address.get_port() == 7000);
    REQUIRE(config.m_username == "user");
    REQUIRE(config.m_password.empty());
  }

  TEST_CASE("parse_special_characters_in_credentials") {
    auto yaml = YAML::Load(R"(
      address: "192.168.1.1:3000"
      username: "user@domain.com"
      password: "p@$$w0rd!"
    )");
    auto config = ServiceLocatorClientConfig::parse(yaml);
    REQUIRE(config.m_address.get_host() == "192.168.1.1");
    REQUIRE(config.m_address.get_port() == 3000);
    REQUIRE(config.m_username == "user@domain.com");
    REQUIRE(config.m_password == "p@$$w0rd!");
  }

  TEST_CASE("parse_missing_address") {
    auto yaml = YAML::Load(R"(
      username: test_user
      password: test_password
    )");
    REQUIRE_THROWS_AS(
      ServiceLocatorClientConfig::parse(yaml), std::runtime_error);
  }

  TEST_CASE("parse_missing_username") {
    auto yaml = YAML::Load(R"(
      address: "192.168.1.100:8080"
      password: test_password
    )");
    REQUIRE_THROWS_AS(
      ServiceLocatorClientConfig::parse(yaml), std::runtime_error);
  }

  TEST_CASE("parse_missing_password") {
    auto yaml = YAML::Load(R"(
      address: "192.168.1.100:8080"
      username: test_user
    )");
    REQUIRE_THROWS_AS(
      ServiceLocatorClientConfig::parse(yaml), std::runtime_error);
  }

  TEST_CASE("parse_empty_yaml") {
    auto yaml = YAML::Load("");
    REQUIRE_THROWS_AS(
      ServiceLocatorClientConfig::parse(yaml), std::runtime_error);
  }
}

TEST_SUITE("ServiceConfiguration") {
  TEST_CASE("parse_with_default_name") {
    auto yaml = YAML::Load(R"(
      interface: "0.0.0.0:9000"
    )");
    auto config = ServiceConfiguration::parse(yaml, "default_service");
    REQUIRE(config.m_name == "default_service");
    REQUIRE(config.m_interface.get_host() == "0.0.0.0");
    REQUIRE(config.m_interface.get_port() == 9000);
    auto addresses_value = config.m_properties.get("addresses");
    REQUIRE(addresses_value.has_value());
    auto addresses_string = get<std::string>(*addresses_value);
    REQUIRE(addresses_string.find("0.0.0.0:9000") != std::string::npos);
  }

  TEST_CASE("parse_with_explicit_service_name") {
    auto yaml = YAML::Load(R"(
      service: custom_service
      interface: "192.168.1.1:8080"
    )");
    auto config = ServiceConfiguration::parse(yaml, "default_service");
    REQUIRE(config.m_name == "custom_service");
    REQUIRE(config.m_interface.get_host() == "192.168.1.1");
    REQUIRE(config.m_interface.get_port() == 8080);
  }

  TEST_CASE("parse_with_single_address") {
    auto yaml = YAML::Load(R"(
      service: test_service
      interface: "10.0.0.1:5000"
      addresses:
        - "10.0.0.2:5000"
    )");
    auto config = ServiceConfiguration::parse(yaml, "default");
    REQUIRE(config.m_name == "test_service");
    REQUIRE(config.m_interface.get_host() == "10.0.0.1");
    REQUIRE(config.m_interface.get_port() == 5000);
    auto addresses_value = config.m_properties.get("addresses");
    REQUIRE(addresses_value.has_value());
    auto addresses_string = get<std::string>(*addresses_value);
    REQUIRE(addresses_string.find("10.0.0.2:5000") != std::string::npos);
  }

  TEST_CASE("parse_with_multiple_addresses") {
    auto yaml = YAML::Load(R"(
      service: multi_service
      interface: "172.16.0.1:7000"
      addresses:
        - "172.16.0.2:7000"
        - "172.16.0.3:7000"
        - "172.16.0.4:7000"
    )");
    auto config = ServiceConfiguration::parse(yaml, "default");
    REQUIRE(config.m_name == "multi_service");
    REQUIRE(config.m_interface.get_host() == "172.16.0.1");
    REQUIRE(config.m_interface.get_port() == 7000);
    auto addresses_value = config.m_properties.get("addresses");
    REQUIRE(addresses_value.has_value());
    auto addresses_string = get<std::string>(*addresses_value);
    REQUIRE(addresses_string.find("172.16.0.2:7000") != std::string::npos);
    REQUIRE(addresses_string.find("172.16.0.3:7000") != std::string::npos);
    REQUIRE(addresses_string.find("172.16.0.4:7000") != std::string::npos);
  }

  TEST_CASE("parse_with_localhost_interface") {
    auto yaml = YAML::Load(R"(
      service: local_service
      interface: "localhost:3000"
    )");
    auto config = ServiceConfiguration::parse(yaml, "default");
    REQUIRE(config.m_name == "local_service");
    REQUIRE(config.m_interface.get_host() == "localhost");
    REQUIRE(config.m_interface.get_port() == 3000);
  }

  TEST_CASE("parse_missing_interface") {
    auto yaml = YAML::Load(R"(
      service: no_interface_service
    )");
    REQUIRE_THROWS_AS(
      ServiceConfiguration::parse(yaml, "default"), std::runtime_error);
  }

  TEST_CASE("parse_wildcard_interface") {
    auto yaml = YAML::Load(R"(
      service: wildcard_service
      interface: "0.0.0.0:12345"
    )");
    auto config = ServiceConfiguration::parse(yaml, "default");
    REQUIRE(config.m_name == "wildcard_service");
    REQUIRE(config.m_interface.get_host() == "0.0.0.0");
    REQUIRE(config.m_interface.get_port() == 12345);
  }
}
