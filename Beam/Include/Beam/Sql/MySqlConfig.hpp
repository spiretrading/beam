#ifndef BEAM_MYSQL_CONFIG_HPP
#define BEAM_MYSQL_CONFIG_HPP
#include <string>
#include <vector>
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

namespace Beam {

  /** Stores the configuration needed to connect a MySQL database. */
  struct MySqlConfig {

    /**
     * Parses a MySqlConfig from a YAML Node.
     * @param node The YAML node to parse.
     */
    static MySqlConfig parse(const YAML::Node& config);

    /**
     * Parses multiple MySqlConfigs from a YAML Node for use in a replicated
     * database.
     * @param node The YAML node to parse.
     */
    static std::vector<MySqlConfig> parse_replication(const YAML::Node& config);

    /** The database's IP address. */
    IpAddress m_address;

    /** The username. */
    std::string m_username;

    /** The password. */
    std::string m_password;

    /** The schema to use. */
    std::string m_schema;
  };

  inline MySqlConfig MySqlConfig::parse(const YAML::Node& node) {
    auto config = MySqlConfig();
    config.m_address = extract<IpAddress>(node, "address");
    config.m_username = extract<std::string>(node, "username");
    config.m_password = extract<std::string>(node, "password");
    config.m_schema = extract<std::string>(node, "schema");
    return config;
  }

  inline std::vector<MySqlConfig>
      MySqlConfig::parse_replication(const YAML::Node& node) {
    auto config = std::vector<MySqlConfig>();
    if(node.Type() != YAML::NodeType::Sequence) {
      auto primary_config = parse(node);
      config.push_back(primary_config);
    } else {
      for(auto& sub_node : node) {
        if(config.empty()) {
          auto primary_config = parse(sub_node);
          config.push_back(primary_config);
        } else {
          auto instance = MySqlConfig();
          instance.m_address =
            extract<IpAddress>(sub_node, "address", config.back().m_address);
          instance.m_username = extract<std::string>(
            sub_node, "username", config.back().m_username);
          instance.m_password = extract<std::string>(
            sub_node, "password", config.back().m_password);
          instance.m_schema =
            extract<std::string>(sub_node, "schema", config.back().m_schema);
          config.push_back(instance);
        }
      }
    }
    return config;
  }
}

#endif
