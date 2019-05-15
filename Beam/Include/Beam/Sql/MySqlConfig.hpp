#ifndef BEAM_MYSQL_CONFIG_HPP
#define BEAM_MYSQL_CONFIG_HPP
#include <string>
#include <vector>
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Sql/Sql.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

namespace Beam {

  /** Stores the configuration needed to connect a MySQL database. */
  struct MySqlConfig {

    //! Parses a MySqlConfig from a YAML Node.
    /*!
      \param node The YAML node to parse.
    */
    static MySqlConfig Parse(const YAML::Node& config);

    //! Parses multiple MySqlConfigs from a YAML Node for use in a replicated
    //! database.
    /*!
      \param node The YAML node to parse.
    */
    static std::vector<MySqlConfig> ParseReplication(const YAML::Node& config);

    //! The database's IP address.
    Network::IpAddress m_address;

    //! The username.
    std::string m_username;

    //! The password.
    std::string m_password;

    //! The schema to use.
    std::string m_schema;
  };

  inline MySqlConfig MySqlConfig::Parse(const YAML::Node& node) {
    auto config = MySqlConfig();
    config.m_address = Extract<Network::IpAddress>(node, "address");
    config.m_username = Extract<std::string>(node, "username");
    config.m_password = Extract<std::string>(node, "password");
    config.m_schema = Extract<std::string>(node, "schema");
    return config;
  }

  inline std::vector<MySqlConfig> MySqlConfig::ParseReplication(
      const YAML::Node& node) {
    auto config = std::vector<MySqlConfig>();
    if(node.Type() != YAML::NodeType::Sequence) {
      auto primaryConfig = Parse(node);
      config.push_back(primaryConfig);
    } else {
      for(auto& subNode : node) {
        if(config.empty()) {
          auto primaryConfig = Parse(subNode);
          config.push_back(primaryConfig);
        } else {
          auto instance = MySqlConfig();
          instance.m_address = Extract<Network::IpAddress>(subNode, "address",
            config.back().m_address);
          instance.m_username = Extract<std::string>(subNode, "username",
            config.back().m_username);
          instance.m_password = Extract<std::string>(subNode, "password",
            config.back().m_password);
          instance.m_schema = Extract<std::string>(subNode, "schema",
            config.back().m_schema);
          config.push_back(instance);
        }
      }
    }
    return config;
  }
}

#endif
