#ifndef BEAM_YAML_CONFIG_HPP
#define BEAM_YAML_CONFIG_HPP
#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/rational.hpp>
#include <boost/throw_exception.hpp>
#include <tclap/CmdLine.h>
#include <yaml-cpp/yaml.h>
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Parsers/RationalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/SerializedValue.hpp"
#include "Beam/Utilities/AssertionException.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {

  /**
   * Template used to parse a value from a YAML node.
   * @tparam T The type of value to parse.
   */
  template<typename T>
  struct YamlValueExtractor {

    /**
     * Parses a YAML node.
     * @param node The node to parse.
     * @return The parsed value.
     */
    T operator ()(const YAML::Node& node) const {
      auto symbol = node.as<std::string>();
      boost::trim(symbol);
      auto value = SerializedValue<T>();
      value.initialize();
      auto stream = std::stringstream(symbol);
      stream >> *value;
      return *value;
    }
  };

  /**
   * Loads a YAML Node from a file.
   * @param path The path to the YAML file.
   * @return The YAML Node represented by the file at the specified <i>path</i>.
   */
  inline YAML::Node load_file(std::string_view path) {
    auto config_stream = std::ifstream(path.data());
    if(!config_stream.good()) {
      auto message = std::stringstream();
      message << "YAML file not found: " << path << '\n';
      boost::throw_with_location(std::runtime_error(message.str()));
    }
    try {
      return YAML::Load(config_stream);
    } catch(const YAML::ParserException& e) {
      auto message = std::stringstream();
      message << "Invalid YAML in file \"" << path << "\" at line " <<
        (e.mark.line + 1) << ", " << "column " << (e.mark.column + 1) << ": " <<
        e.msg << '\n';
      boost::throw_with_location(std::runtime_error(message.str()));
    }
  }

  /**
   * Parses a YAML node based on command line arguments.
   * @param argc The number of command line arguments.
   * @param argv The array of command line arguments.
   * @param version_tag The version tag to display.
   * @return The YAML node parsed based on the command line arguments.
   */
  inline YAML::Node parse_command_line(
      int argc, const char** argv, std::string_view version_tag) {
    try {
      auto cmd = TCLAP::CmdLine("", ' ', version_tag.data());
      auto config_arg = TCLAP::ValueArg<std::string>(
        "c", "config", "Configuration file", false, "config.yml", "path");
      cmd.add(config_arg);
      cmd.parse(argc, argv);
      return load_file(config_arg.getValue());
    } catch(const TCLAP::ArgException& e) {
      boost::throw_with_location(
        std::runtime_error("Error parsing command line: " + e.error() +
          " for argument " + e.argId()));
    }
  }

  /**
   * Returns an exception with a message pointing to the position of the error.
   * @param message The error message.
   * @param mark The location of the error.
   * @return A runtime exception with the specified <i>message</i>.
   */
  inline std::runtime_error make_yaml_parser_exception(
      std::string_view message, const YAML::Mark& mark) {
    auto ss = std::stringstream();
    ss << "Parser error at line " << (mark.line + 1) << ", " << "column " <<
      (mark.column + 1) << ": " << message << "\n";
    return std::runtime_error(ss.str());
  }

  /**
   * Extracts a required value from a YAML node.
   * @param node The YAML node to extract from.
   * @return The value represented by the <i>node</i>.
   */
  template<typename T>
  T extract(const YAML::Node& node) {
    return YamlValueExtractor<T>()(node);
  }

  /**
   * Extracts a required value from a YAML node.
   * @param node The YAML node to extract from.
   * @param name The name of the value to extract.
   * @return The value with the specified <i>name</i>.
   */
  template<typename T>
  T extract(const YAML::Node& node, const std::string& name) {
    auto& value_node = node[name];
    BEAM_ASSERT_MESSAGE(value_node.IsDefined(), "Config error at line " <<
      (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
      ":\n\tNode not found: " << name << std::endl);
    return extract<T>(value_node);
  }

  /**
   * Extracts an optional value from a YAML node or returns a default value.
   * @param node The YAML node to extract from.
   * @param name The name of the value to extract.
   * @param d The default value to return if the value is not found.
   * @return The value with the specified <i>name</i> or <i>d</i> iff the
   *         value is not found.
   */
  template<typename T>
  T extract(const YAML::Node& node, const std::string& name, const T& d) {
    auto& value_node = node[name];
    if(!value_node) {
      return d;
    }
    return extract<T>(node, name);
  }

  /**
   * Returns a YAML Node with a specified name from a parent Node.
   * @param node The parent YAML Node.
   * @param name The name of the YAML child Node to extract.
   * @return The child Node with the specified <i>name</i>.
   */
  inline YAML::Node get_node(const YAML::Node& node, const std::string& name) {
    auto value_node = node[name];
    if(value_node) {
      return value_node;
    }
    BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
      (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
      ":\n\tNode not found: " << name << std::endl);
  }

  template<>
  struct YamlValueExtractor<std::string> {
    std::string operator ()(const YAML::Node& node) const {
      auto value = node.as<std::string>();
      boost::trim(value);
      return value;
    }
  };

  template<>
  struct YamlValueExtractor<bool> {
    bool operator ()(const YAML::Node& node) const {
      return node.as<bool>();
    }
  };

  template<>
  struct YamlValueExtractor<boost::rational<int>> {
    boost::rational<int> operator ()(const YAML::Node& node) const {
      return parse(RationalParser<int>(), node.as<std::string>());
    }
  };

  template<typename T>
  struct YamlValueExtractor<std::vector<T>> {
    std::vector<T> operator ()(const YAML::Node& node) const {
      auto values = std::vector<T>();
      for(auto& i : node) {
        values.push_back(extract<T>(i));
      }
      return values;
    }
  };

  template<>
  struct YamlValueExtractor<boost::posix_time::time_duration> {
    boost::posix_time::time_duration operator ()(const YAML::Node& node) const {
      auto raw_value = node.as<std::string>();
      if(raw_value == "infinity" || raw_value == "+infinity") {
        return boost::posix_time::pos_infin;
      } else if(raw_value == "-infinity") {
        return boost::posix_time::neg_infin;
      }
      auto value = boost::posix_time::time_duration();
      auto i = raw_value.crbegin();
      while(i != raw_value.rend() && std::isalpha(*i)) {
        ++i;
      }
      BEAM_ASSERT_MESSAGE(i != raw_value.rend(), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid time duration specified." << std::endl);
      auto unit_offset = std::distance(i, raw_value.crend());
      auto scalar_value = int();
      try {
        auto config_scalar = boost::algorithm::trim_copy(
          raw_value.substr(0, unit_offset));
        scalar_value = boost::lexical_cast<int>(config_scalar);
      } catch(const std::exception& e) {
        BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
          (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
          ":\n\t" << e.what() << std::endl);
      }
      auto unit = boost::algorithm::trim_copy(raw_value.substr(unit_offset));
      if(unit == "h") {
        return boost::posix_time::hours(scalar_value);
      } else if(unit == "m") {
        return boost::posix_time::minutes(scalar_value);
      } else if(unit == "s") {
        return boost::posix_time::seconds(scalar_value);
      } else if(unit == "ms") {
        return boost::posix_time::milliseconds(scalar_value);
      } else if(unit == "us") {
        return boost::posix_time::microseconds(scalar_value);
      }
      BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid time unit given:\n\t" << unit << std::endl);
    }
  };

  template<>
  struct YamlValueExtractor<boost::gregorian::date> {
    boost::gregorian::date operator ()(const YAML::Node& node) const {
      auto value = boost::gregorian::date();
      auto parser = date_parser();
      auto source = to_parser_stream(node.as<std::string>());
      BEAM_ASSERT_MESSAGE(parser.read(source, value), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid date specified." << std::endl);
      return value;
    }
  };

  template<>
  struct YamlValueExtractor<boost::posix_time::ptime> {
    boost::posix_time::ptime operator ()(const YAML::Node& node) const {
      auto value = boost::posix_time::ptime();
      auto parser = date_time_parser();
      auto source = to_parser_stream(node.as<std::string>());
      BEAM_ASSERT_MESSAGE(parser.read(source, value), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid date/time specified." << std::endl);
      return value;
    }
  };

  template<>
  struct YamlValueExtractor<IpAddress> {
    IpAddress operator ()(const YAML::Node& node) const {
      auto raw_value = node.as<std::string>();
      boost::trim(raw_value);
      auto colon_position = raw_value.find(':');
      if(colon_position == std::string::npos) {
        return IpAddress(raw_value, 0);
      }
      auto host = raw_value.substr(0, colon_position);
      auto port = std::make_unsigned_t<short>();
      try {
        port = boost::lexical_cast<unsigned short>(
          raw_value.substr(colon_position + 1));
      } catch(const std::exception& e) {
        BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
          (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
          ":\n\t" << e.what() << std::endl);
      }
      return IpAddress(host, port);
    }
  };
}

#endif
