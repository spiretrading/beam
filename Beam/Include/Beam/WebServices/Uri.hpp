#ifndef BEAM_URI_HPP
#define BEAM_URI_HPP
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <format>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {

  /** Represents a uniform resource identifier. */
  class Uri {
    public:

      /** Constructs an empty URI. */
      Uri() noexcept;

      /**
       * Constructs a URI from a string.
       * @param source The string to parse the URI from.
       */
      Uri(std::string_view source);

      /**
       * Constructs a URI from a string.
       * @param source The string to parse the URI from.
       */
      Uri(const char* source);

      /**
       * Constructs a URI from a sequence.
       * @param first The character to parse.
       * @param last One past the last character to parse.
       */
      Uri(const char* first, const char* last);

      /** Returns the scheme. */
      const std::string& get_scheme() const;

      /** Returns the username. */
      const std::string& get_username() const;

      /** Returns the password. */
      const std::string& get_password() const;

      /** Returns the hostname. */
      const std::string& get_hostname() const;

      /** Returns the port. */
      unsigned short get_port() const;

      /** Sets the port. */
      void set_port(unsigned short port);

      /** Returns the path. */
      const std::string& get_path() const;

      /** Returns the query. */
      const std::string& get_query() const;

      /** Returns the fragment. */
      const std::string& get_fragment() const;

    private:
      std::string m_scheme;
      std::string m_username;
      std::string m_password;
      std::string m_hostname;
      unsigned short m_port;
      std::string m_path;
      std::string m_query;
      std::string m_fragment;
  };

  /** Signals an invalid/malformed URI. */
  class MalformedUriException : public std::runtime_error {
    public:

      /** Constructs a MalformedUriException. */
      MalformedUriException();

      /**
       * Constructs a MalformedUriException.
       * @param message A message describing the error.
       */
      explicit MalformedUriException(const std::string& message);
  };

  /**
   * URI-encodes a string.
   * @param source The string to encode.
   * @return The URI-encoded string.
   */
  inline std::string uri_encode(std::string_view source) {
    static constexpr auto HEX_DIGITS = "0123456789ABCDEF";
    auto result = std::string();
    result.reserve(source.size());
    for(auto c : source) {
      if(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ||
          c == '.' || c == '~') {
        result += c;
      } else {
        auto byte = static_cast<unsigned char>(c);
        result += '%';
        result += HEX_DIGITS[byte >> 4];
        result += HEX_DIGITS[byte & 0x0F];
      }
    }
    return result;
  }

  /**
   * Decodes a URI-encoded string.
   * @param source The URI-encoded string.
   * @return The decoded string.
   */
  inline std::string uri_decode(std::string_view source) {
    static constexpr auto HEX_VALUES = [] {
      auto table = std::array<int, 256>();
      table.fill(-1);
      for(auto i = 0; i <= 9; ++i) {
        table['0' + i] = i;
      }
      for(auto i = 0; i < 6; ++i) {
        table['A' + i] = 10 + i;
        table['a' + i] = 10 + i;
      }
      return table;
    }();
    auto result = std::string();
    result.reserve(source.size());
    for(auto i = std::size_t(0); i < source.size(); ++i) {
      if(source[i] == '%' && i + 2 < source.size()) {
        auto high = HEX_VALUES[static_cast<unsigned char>(source[i + 1])];
        auto low = HEX_VALUES[static_cast<unsigned char>(source[i + 2])];
        if(high >= 0 && low >= 0) {
          result += static_cast<char>(high << 4 | low);
          i += 2;
          continue;
        }
      }
      if(source[i] == '+') {
        result += ' ';
      } else {
        result += source[i];
      }
    }
    return result;
  }

  /**
   * Parses query parameters from a raw query string.
   * @param query The raw query string (without the leading '?').
   * @return A multimap of decoded parameter names to decoded values.
   */
  inline std::unordered_multimap<std::string, std::string> parse_query(
      std::string_view query) {
    auto parameters = std::unordered_multimap<std::string, std::string>();
    auto position = std::string_view::size_type(0);
    while(position < query.size()) {
      auto separator = query.find('&', position);
      if(separator == std::string_view::npos) {
        separator = query.size();
      }
      auto pair = query.substr(position, separator - position);
      if(!pair.empty()) {
        auto equals = pair.find('=');
        if(equals == std::string_view::npos) {
          parameters.emplace(uri_decode(pair), std::string());
        } else {
          parameters.emplace(
            uri_decode(pair.substr(0, equals)),
            uri_decode(pair.substr(equals + 1)));
        }
      }
      position = separator + 1;
    }
    return parameters;
  }

  /**
   * Parses query parameters from a raw query string.
   * @param query The raw query string (without the leading '?').
   * @return A multimap of decoded parameter names to decoded values.
   */
  inline std::unordered_multimap<std::string, std::string> parse_query(
      const char* query) {
    return parse_query(std::string_view(query));
  }

  /**
   * Parses query parameters from a URI.
   * @param uri The URI to extract query parameters from.
   * @return A multimap of decoded parameter names to decoded values.
   */
  inline std::unordered_multimap<std::string, std::string> parse_query(
      const Uri& uri) {
    return parse_query(uri.get_query());
  }

  inline std::ostream& operator <<(std::ostream& sink, const Uri& uri) {
    if(!uri.get_scheme().empty()) {
      sink << uri.get_scheme() << ':';
    }
    if(!uri.get_hostname().empty()) {
      sink << '/' << '/';
    }
    if(!uri.get_username().empty()) {
      sink << uri.get_username() << ':' << uri.get_password() << '@';
    } else if(!uri.get_password().empty()) {
      sink << uri.get_username() << ':' << uri.get_password() << '@';
    }
    sink << uri.get_hostname();
    auto port = uri.get_port();
    auto is_default_port =
      port == 80 && (uri.get_scheme() == "http" || uri.get_scheme() == "ws") ||
      port == 443 && (uri.get_scheme() == "https" || uri.get_scheme() == "wss");
    if(port != 0 && !is_default_port) {
      sink << ':' << port;
    }
    sink << uri.get_path();
    if(!uri.get_query().empty()) {
      sink << '?' << uri.get_query();
    }
    if(!uri.get_fragment().empty()) {
      sink << '#' << uri.get_fragment();
    }
    return sink;
  }

  inline Uri::Uri() noexcept
    : m_port(0) {}

  inline Uri::Uri(std::string_view source)
    : Uri(source.data(), source.data() + source.size()) {}

  inline Uri::Uri(const char* source)
    : Uri(std::string_view(source)) {}

  inline Uri::Uri(const char* first, const char* last)
      : m_port(0) {
    auto source = std::string_view(first, last);
    auto is_word = [] (auto c) {
      return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };
    auto scheme = std::string_view();
    auto username = std::string_view();
    auto password = std::string_view();
    auto hostname = std::string_view();
    auto port_text = std::string_view();
    auto path = std::string_view();
    auto query = std::string_view();
    auto fragment = std::string_view();
    auto parse_tail = [&] (std::string_view tail) {
      if(tail.starts_with("//")) {
        return false;
      }
      auto end = std::min(tail.find_first_of("?#"), tail.size());
      path = tail.substr(0, end);
      query = std::string_view();
      fragment = std::string_view();
      tail.remove_prefix(end);
      if(tail.starts_with('?')) {
        tail.remove_prefix(1);
        end = std::min(tail.find('#'), tail.size());
        if(end == 0) {
          return false;
        }
        query = tail.substr(0, end);
        tail.remove_prefix(end);
      }
      if(tail.starts_with('#')) {
        tail.remove_prefix(1);
        if(!std::ranges::all_of(tail, is_word)) {
          return false;
        }
        fragment = tail;
      }
      return true;
    };
    auto parse_authority = [&] (std::string_view authority) {
      auto end = std::min(authority.find_first_of("/?:"), authority.size());
      hostname = authority.substr(0, end);
      port_text = std::string_view();
      auto tail = authority.substr(end);
      if(tail.starts_with(':')) {
        auto port_end = std::size_t(1);
        while(port_end < tail.size() &&
            tail[port_end] >= '0' && tail[port_end] <= '9') {
          ++port_end;
        }
        if(port_end > 1) {
          port_text = tail.substr(1, port_end - 1);
          if(parse_tail(tail.substr(port_end))) {
            return true;
          }
          if(port_end > 2) {
            port_text.remove_suffix(1);
            return parse_tail(tail.substr(port_end - 1));
          }
          port_text = std::string_view();
        }
      }
      if(parse_tail(tail)) {
        return true;
      }
      if(end != 0) {
        hostname.remove_suffix(1);
        return parse_tail(authority.substr(end - 1));
      }
      return false;
    };
    auto parse = [&] (std::string_view remainder) {
      username = std::string_view();
      password = std::string_view();
      hostname = std::string_view();
      port_text = std::string_view();
      if(!remainder.starts_with("//")) {
        return parse_tail(remainder);
      }
      remainder.remove_prefix(2);
      auto username_end = std::size_t(0);
      while(username_end < remainder.size() &&
          is_word(remainder[username_end])) {
        ++username_end;
      }
      if(username_end < remainder.size() && remainder[username_end] == ':') {
        auto password_end = username_end + 1;
        while(password_end < remainder.size() &&
            is_word(remainder[password_end])) {
          ++password_end;
        }
        if(password_end < remainder.size() && remainder[password_end] == '@') {
          username = remainder.substr(0, username_end);
          password = remainder.substr(
            username_end + 1, password_end - username_end - 1);
          if(parse_authority(remainder.substr(password_end + 1))) {
            return true;
          }
          username = std::string_view();
          password = std::string_view();
        }
      }
      return parse_authority(remainder);
    };
    auto scheme_end = std::size_t(0);
    while(scheme_end < source.size() && is_word(source[scheme_end])) {
      ++scheme_end;
    }
    auto matched = false;
    if(scheme_end != 0 && scheme_end < source.size() &&
        source[scheme_end] == ':') {
      scheme = source.substr(0, scheme_end);
      matched = parse(source.substr(scheme_end + 1));
    }
    if(!matched) {
      scheme = std::string_view();
      matched = parse(source);
    }
    if(!matched) {
      boost::throw_with_location(
        MalformedUriException("URI is not well formed."));
    }
    auto port = std::size_t(0);
    for(auto c : port_text) {
      port = port * 10 + (c - '0');
      if(port > std::numeric_limits<unsigned short>::max()) {
        boost::throw_with_location(
          MalformedUriException("URI is not well formed."));
      }
    }
    m_port = static_cast<unsigned short>(port);
    m_scheme = scheme;
    m_username = username;
    m_password = password;
    m_hostname = hostname;
    m_path = path;
    m_query = query;
    m_fragment = fragment;
  }

  inline const std::string& Uri::get_scheme() const {
    return m_scheme;
  }

  inline const std::string& Uri::get_username() const {
    return m_username;
  }

  inline const std::string& Uri::get_password() const {
    return m_password;
  }

  inline const std::string& Uri::get_hostname() const {
    return m_hostname;
  }

  inline unsigned short Uri::get_port() const {
    if(m_port == 0) {
      if(get_scheme() == "http" || get_scheme() == "ws") {
        return static_cast<std::uint16_t>(80);
      } else if(get_scheme() == "https" || get_scheme() == "wss") {
        return static_cast<std::uint16_t>(443);
      }
    }
    return m_port;
  }

  inline void Uri::set_port(unsigned short port) {
    m_port = port;
  }

  inline const std::string& Uri::get_path() const {
    return m_path;
  }

  inline const std::string& Uri::get_query() const {
    return m_query;
  }

  inline const std::string& Uri::get_fragment() const {
    return m_fragment;
  }

  inline MalformedUriException::MalformedUriException()
    : MalformedUriException("Malformed URI.") {}

  inline MalformedUriException::MalformedUriException(
    const std::string& message)
    : std::runtime_error(message) {}

  template<>
  inline constexpr auto is_structure<Uri> = false;

  template<>
  struct Send<Uri> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, const Uri& value) const {
      sender.send(name, to_string(value));
    }
  };

  template<>
  struct Receive<Uri> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, Uri& value) const {
      value = Uri(receive<std::string>(receiver, name));
    }
  };
}

template<>
class std::formatter<Beam::Uri> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Beam::Uri& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        Beam::to_string(value), context);
    }
};

#endif
