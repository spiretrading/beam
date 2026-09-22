#ifndef BEAM_YAML_CONFIG_HPP
#define BEAM_YAML_CONFIG_HPP
#include <algorithm>
#include <charconv>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
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
namespace Details {
  inline std::string decode_yaml_pointer(std::string_view fragment) {
    auto pointer = std::string();
    for(auto i = std::size_t(0); i != fragment.size(); ++i) {
      if(fragment[i] != '%') {
        pointer += fragment[i];
      } else {
        if(fragment.size() - i < 3) {
          throw std::runtime_error("Invalid JSON Pointer percent escape.");
        }
        auto value = unsigned();
        auto begin = fragment.data() + i + 1;
        auto result = std::from_chars(begin, begin + 2, value, 16);
        if(result.ec != std::errc() || result.ptr != begin + 2) {
          throw std::runtime_error("Invalid JSON Pointer percent escape.");
        }
        pointer += static_cast<char>(value);
        i += 2;
      }
    }
    return pointer;
  }

  inline YAML::Node select_yaml_node(
      YAML::Node node, std::string_view pointer) {
    if(pointer.empty()) {
      return node;
    }
    if(pointer.front() != '/') {
      throw std::runtime_error("JSON Pointer must begin with '/'.");
    }
    auto begin = std::size_t(1);
    while(true) {
      auto end = pointer.find('/', begin);
      if(end == std::string_view::npos) {
        end = pointer.size();
      }
      auto token = std::string();
      for(auto i = begin; i != end; ++i) {
        if(pointer[i] != '~') {
          token += pointer[i];
        } else {
          ++i;
          if(i == end || (pointer[i] != '0' && pointer[i] != '1')) {
            throw std::runtime_error("Invalid JSON Pointer '~' escape.");
          }
          if(pointer[i] == '0') {
            token += '~';
          } else {
            token += '/';
          }
        }
      }
      if(node.IsMap()) {
        auto selected = YAML::Node(YAML::NodeType::Undefined);
        for(auto entry : node) {
          if(entry.first.IsScalar() && entry.first.Scalar() == token) {
            if(selected.IsDefined()) {
              throw std::runtime_error("Ambiguous JSON Pointer key: " + token);
            }
            selected.reset(entry.second);
          }
        }
        if(!selected.IsDefined()) {
          throw std::runtime_error("JSON Pointer key not found: " + token);
        }
        node.reset(selected);
      } else if(node.IsSequence()) {
        auto index = std::size_t();
        auto result = std::from_chars(
          token.data(), token.data() + token.size(), index);
        if(token.empty() || (token.size() > 1 && token.front() == '0') ||
            result.ec != std::errc() ||
            result.ptr != token.data() + token.size() || index >= node.size()) {
          throw std::runtime_error("Invalid JSON Pointer index: " + token);
        }
        node.reset(std::as_const(node)[index]);
      } else {
        throw std::runtime_error("JSON Pointer traverses a scalar or null.");
      }
      if(end == pointer.size()) {
        return node;
      }
      begin = end + 1;
    }
  }

  inline YAML::Node merge_yaml_nodes(const YAML::Node& defaults,
      const YAML::Node& overrides,
      std::vector<std::pair<YAML::Node, YAML::Node>>& ancestors) {
    if(std::any_of(ancestors.begin(), ancestors.end(), [&] (const auto& pair) {
        return pair.first.is(defaults) && pair.second.is(overrides);
      })) {
      throw std::runtime_error("Circular YAML mapping merge.");
    }
    ancestors.emplace_back(defaults, overrides);
    auto merged = YAML::Clone(defaults);
    for(auto entry : overrides) {
      auto key = entry.first.as<std::string>();
      auto inherited = defaults[key];
      auto value = YAML::Node();
      if(inherited && inherited.IsMap() && entry.second.IsMap()) {
        value = merge_yaml_nodes(inherited, entry.second, ancestors);
      } else {
        value = YAML::Clone(entry.second);
      }
      merged.remove(key);
      merged[key] = value;
    }
    ancestors.pop_back();
    return merged;
  }

  template<typename L>
  class YamlConfigLoader {
    public:
      explicit YamlConfigLoader(L loader) noexcept(
        std::is_nothrow_move_constructible_v<L>)
        : m_loader(std::move(loader)) {}

      YAML::Node load(const std::filesystem::path& path,
          std::string_view fragment) {
        try {
          auto file = std::filesystem::absolute(path).lexically_normal();
          auto pointer = decode_yaml_pointer(fragment);
          auto identity = std::pair(
            std::filesystem::weakly_canonical(file), pointer);
          if(std::find(m_paths.begin(), m_paths.end(), identity) !=
              m_paths.end()) {
            throw std::runtime_error("Circular YAML include.");
          }
          m_paths.push_back(identity);
          try {
            auto node = select_yaml_node(m_loader(file), pointer);
            auto ancestors = std::vector<YAML::Node>();
            resolve(node, file, ancestors);
            m_paths.pop_back();
            return node;
          } catch(...) {
            m_paths.pop_back();
            throw;
          }
        } catch(const std::exception& e) {
          auto message = std::stringstream();
          message << "Unable to load YAML \"" << path.string();
          if(!fragment.empty()) {
            message << '#' << fragment;
          }
          message << "\":\n" << e.what();
          boost::throw_with_location(std::runtime_error(message.str()));
        }
      }

    private:
      L m_loader;
      std::vector<std::pair<std::filesystem::path, std::string>> m_paths;

      void resolve(YAML::Node node, const std::filesystem::path& path,
          std::vector<YAML::Node>& ancestors) {
        if(node.Tag() == "!include") {
          if(!node.IsScalar() || node.Scalar().empty()) {
            throw std::runtime_error("!include requires a nonempty path.");
          }
          auto reference = node.Scalar();
          auto separator = reference.find('#');
          auto file = path;
          if(separator != 0) {
            file = path.parent_path() / reference.substr(0, separator);
          }
          auto fragment = std::string_view();
          if(separator != std::string::npos) {
            fragment = std::string_view(reference).substr(separator + 1);
          }
          node = load(file, fragment);
        } else if(node.IsMap() || node.IsSequence()) {
          if(std::any_of(ancestors.begin(), ancestors.end(),
              [&] (const auto& ancestor) { return node.is(ancestor); })) {
            return;
          }
          ancestors.push_back(node);
          if(node.IsMap()) {
            auto source = YAML::Node(YAML::NodeType::Undefined);
            for(auto entry : node) {
              resolve(entry.second, path, ancestors);
              if(entry.first.IsScalar() && entry.first.Scalar() == "<<" &&
                  (entry.first.Tag() == "?" ||
                    entry.first.Tag() == "tag:yaml.org,2002:merge")) {
                if(source.IsDefined()) {
                  throw std::runtime_error("Multiple YAML merge keys.");
                }
                source.reset(entry.second);
              }
            }
            if(source.IsDefined()) {
              if(!source.IsMap()) {
                throw std::runtime_error(
                  "YAML merge source must be a mapping.");
              }
              if(std::any_of(ancestors.begin(), ancestors.end(),
                  [&] (const auto& ancestor) { return source.is(ancestor); })) {
                throw std::runtime_error("Circular YAML mapping merge.");
              }
              node.remove("<<");
              auto merges =
                std::vector<std::pair<YAML::Node, YAML::Node>>();
              node = merge_yaml_nodes(source, node, merges);
            }
          } else {
            for(auto i = std::size_t(0); i != node.size(); ++i) {
              resolve(node[i], path, ancestors);
            }
          }
          ancestors.pop_back();
        }
      }
  };
}

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
      auto is_valid = !stream.fail();
      stream >> std::ws;
      BEAM_ASSERT_MESSAGE(is_valid && stream.eof(), "Config error at line " <<
        (node.Mark().line + 1) << ", column " << (node.Mark().column + 1) <<
        ":\n\tInvalid value specified." << std::endl);
      return *value;
    }
  };

  /**
   * Loads a YAML Node from a file.
   * @param path The path to the YAML file.
   * @return The YAML Node represented by the file at the specified <i>path</i>.
   */
  inline YAML::Node load_file(std::string_view path) {
    auto loader = Details::YamlConfigLoader([] (const auto& path) {
      auto stream = std::ifstream(path);
      if(!stream.good()) {
        throw std::runtime_error("YAML file not found: " + path.string());
      }
      return YAML::Load(stream);
    });
    return loader.load(std::filesystem::path(path), "");
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
   * Extracts a required value within an inclusive range.
   * @param node The YAML node to extract from.
   * @param name The name of the value to extract.
   * @param minimum The minimum permitted value.
   * @param maximum The maximum permitted value.
   * @return The value with the specified <i>name</i>.
   */
  template<std::totally_ordered T>
  T extract(const YAML::Node& node, const std::string& name, const T& minimum,
      const T& maximum) {
    auto value = extract<T>(node, name);
    BEAM_ASSERT_MESSAGE(minimum <= value && value <= maximum,
      "Config error at line " << (node[name].Mark().line + 1) <<
      ", column " << (node[name].Mark().column + 1) <<
      ":\n\tValue out of range: " << name << std::endl);
    return value;
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
   * Extracts an optional value or its default within an inclusive range.
   * @param node The YAML node to extract from.
   * @param name The name of the value to extract.
   * @param default_value The value to use if the named value is not found.
   * @param minimum The minimum permitted value.
   * @param maximum The maximum permitted value.
   * @return The named value or its default, validated against the range.
   */
  template<std::totally_ordered T>
  T extract(const YAML::Node& node, const std::string& name,
      const T& default_value, const T& minimum, const T& maximum) {
    if(node[name]) {
      return extract<T>(node, name, minimum, maximum);
    }
    BEAM_ASSERT_MESSAGE(minimum <= default_value && default_value <= maximum,
      "Config error at line " << (node.Mark().line + 1) << ", column " <<
      (node.Mark().column + 1) <<
      ":\n\tDefault value out of range: " << name << std::endl);
    return default_value;
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
