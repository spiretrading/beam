#ifndef BEAM_YAML_CONFIG_HPP
#define BEAM_YAML_CONFIG_HPP
#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/rational.hpp>
#include <boost/throw_exception.hpp>
#include <yaml-cpp/yaml.h>
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Parsers/DateTimeParser.hpp"
#include "Beam/Parsers/Parse.hpp"
#include "Beam/Parsers/RationalParser.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/AssertionException.hpp"

namespace Beam {

  /**
   * Template used to parse a value from a YAML node.
   * @param <T> The type of value to parse.
   */
  template<typename T, typename Enabled = void>
  struct YamlValueExtractor {

    /**
     * Parses a YAML node.
     * @param node The node to parse.
     * @return The parsed value.
     */
    T operator ()(const YAML::Node& node) const {
      auto symbol = node.as<std::string>();
      boost::trim(symbol);
      auto value = Beam::Serialization::DefaultConstruct<T>();
      auto stream = std::stringstream(symbol);
      stream >> value;
      return value;
    }
  };

  /**
   * Loads a YAML Node from a file.
   * @param path The path to the YAML file.
   * @return The YAML Node represented by the file at the specified <i>path</i>.
   */
  inline YAML::Node LoadFile(const std::string& path) {
    auto configStream = std::ifstream(path.c_str());
    if(!configStream.good()) {
      auto message = std::stringstream();
      message << "YAML file not found: " << path << "\n";
      BOOST_THROW_EXCEPTION(std::runtime_error(message.str()));
    }
    try {
      return YAML::Load(configStream);
    } catch(const YAML::ParserException& e) {
      auto message = std::stringstream();
      message << "Invalid YAML in file \"" << path << "\" at line " <<
        (e.mark.line + 1) << ", " << "column " << (e.mark.column + 1) << ": " <<
        e.msg << "\n";
      BOOST_THROW_EXCEPTION(std::runtime_error(message.str()));
    }
  }

  /**
   * Returns an exception with a message pointing to the position of the error.
   * @param message The error message.
   * @param mark The location of the error.
   * @return A runtime exception with the specified <i>message</i>.
   */
  inline std::runtime_error MakeYamlParserException(const std::string& message,
      const YAML::Mark& mark) {
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
  T Extract(const YAML::Node& node) {
    return YamlValueExtractor<T>()(node);
  }

  /**
   * Extracts a required value from a YAML node.
   * @param node The YAML node to extract from.
   * @param name The name of the value to extract.
   * @return The value with the specified <i>name</i>.
   */
  template<typename T>
  T Extract(const YAML::Node& node, const std::string& name) {
    auto& valueNode = node[name];
    BEAM_ASSERT_MESSAGE(valueNode.IsDefined(), "Config error at line " <<
      (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
      ":\n\tNode not found: " << name << std::endl);
    return Extract<T>(valueNode);
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
  T Extract(const YAML::Node& node, const std::string& name, const T& d) {
    auto& valueNode = node[name];
    if(!valueNode) {
      return d;
    }
    return Extract<T>(node, name);
  }

  /**
   * Returns a YAML Node with a specified name from a parent Node.
   * @param node The parent YAML Node.
   * @param name The name of the YAML child Node to extract.
   * @return The child Node with the specified <i>name</i>.
   */
  inline YAML::Node GetNode(const YAML::Node& node, const std::string& name) {
    auto valueNode = node[name];
    if(valueNode) {
      return valueNode;
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
      return Parsers::ParseFrom(Parsers::RationalParser<int>(),
        node.as<std::string>());
    }
  };

  template<typename T>
  struct YamlValueExtractor<std::vector<T>> {
    std::vector<T> operator ()(const YAML::Node& node) const {
      auto values = std::vector<T>();
      for(auto& i : node) {
        values.push_back(Extract<T>(i));
      }
      return values;
    }
  };

  template<>
  struct YamlValueExtractor<boost::posix_time::time_duration> {
    boost::posix_time::time_duration operator ()(const YAML::Node& node) const {
      auto rawValue = node.as<std::string>();
      if(rawValue == "infinity" || rawValue == "+infinity") {
        return boost::posix_time::pos_infin;
      } else if(rawValue == "-infinity") {
        return boost::posix_time::neg_infin;
      }
      auto value = boost::posix_time::time_duration();
      auto i = rawValue.crbegin();
      while(i != rawValue.rend() && std::isalpha(*i)) {
        ++i;
      }
      BEAM_ASSERT_MESSAGE(i != rawValue.rend(), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid time duration specified." << std::endl);
      auto unitOffset = std::distance(i, rawValue.crend());
      auto scalarValue = int();
      try {
        auto configScalar = boost::algorithm::trim_copy(
          rawValue.substr(0, unitOffset));
        scalarValue = boost::lexical_cast<int>(configScalar);
      } catch(const std::exception& e) {
        BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
          (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
          ":\n\t" << e.what() << std::endl);
      }
      auto unit = boost::algorithm::trim_copy(rawValue.substr(unitOffset));
      if(unit == "h") {
        return boost::posix_time::hours(scalarValue);
      } else if(unit == "m") {
        return boost::posix_time::minutes(scalarValue);
      } else if(unit == "s") {
        return boost::posix_time::seconds(scalarValue);
      } else if(unit == "ms") {
        return boost::posix_time::milliseconds(scalarValue);
      } else if(unit == "us") {
        return boost::posix_time::microseconds(scalarValue);
      }
      BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid time unit given:\n\t" << unit << std::endl);
    }
  };

  template<>
  struct YamlValueExtractor<boost::gregorian::date> {
    boost::gregorian::date operator ()(const YAML::Node& node) const {
      auto rawValue = node.as<std::string>();
      auto value = boost::gregorian::date();
      auto parser = Parsers::DateParser();
      auto source = Parsers::ParserStreamFromString(rawValue);
      BEAM_ASSERT_MESSAGE(parser.Read(source, value), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid date specified." << std::endl);
      return value;
    }
  };

  template<>
  struct YamlValueExtractor<boost::posix_time::ptime> {
    boost::posix_time::ptime operator ()(const YAML::Node& node) const {
      auto rawValue = node.as<std::string>();
      auto value = boost::posix_time::ptime();
      auto parser = Parsers::DateTimeParser();
      auto source = Parsers::ParserStreamFromString(rawValue);
      BEAM_ASSERT_MESSAGE(parser.Read(source, value), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid date/time specified." << std::endl);
      return value;
    }
  };

  template<>
  struct YamlValueExtractor<Network::IpAddress> {
    Network::IpAddress operator ()(const YAML::Node& node) const {
      auto rawValue = node.as<std::string>();
      boost::trim(rawValue);
      auto colonPosition = rawValue.find(':');
      if(colonPosition == std::string::npos) {
        return Network::IpAddress(rawValue, 0);
      }
      auto host = rawValue.substr(0, colonPosition);
      auto port = std::make_unsigned_t<short>();
      try {
        port = boost::lexical_cast<unsigned short>(
          rawValue.substr(colonPosition + 1));
      } catch(const std::exception& e) {
        BEAM_ASSERT_MESSAGE(false, "Config error at line " <<
          (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
          ":\n\t" << e.what() << std::endl);
      }
      return Network::IpAddress(host, port);
    }
  };
}

#endif
