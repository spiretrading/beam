#ifndef BEAM_URI_HPP
#define BEAM_URI_HPP
#include <cstdint>
#include <limits>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <boost/throw_exception.hpp>

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

  inline Uri::Uri(const char* first, const char* last) {
    static const auto URI_PATTERN = std::regex(
      "^((\\w+):)?(\\/\\/(((\\w*):(\\w*)@)?"
      "([^\\/\\?:]*)(:(\\d+))?))?(\\/?([^\\/\\?#][^\\?#]*)"
      "?)?(\\?([^#]+))?(#(\\w*))?");
    static const auto SCHEME_CAPTURE = 2;
    static const auto USERNAME_CAPTURE = 6;
    static const auto PASSWORD_CAPTURE = 7;
    static const auto HOSTNAME_CAPTURE = 8;
    static const auto PORT_CAPTURE = 10;
    static const auto PATHNAME_CAPTURE = 11;
    static const auto QUERY_CAPTURE = 14;
    static const auto FRAGMENT_CAPTURE = 16;
    auto matcher = std::cmatch();
    if(!std::regex_match(first, last, matcher, URI_PATTERN)) {
      boost::throw_with_location(
        MalformedUriException("URI is not well formed."));
    }
    if(matcher[PORT_CAPTURE].str().empty()) {
      m_port = 0;
    } else {
      auto port = std::stoul(matcher[PORT_CAPTURE].str());
      if(port > std::numeric_limits<unsigned short>::max()) {
        boost::throw_with_location(
          MalformedUriException("URI is not well formed."));
      } else {
        m_port = static_cast<unsigned short>(port);
      }
    }
    m_scheme = matcher[SCHEME_CAPTURE];
    m_username = matcher[USERNAME_CAPTURE];
    m_password = matcher[PASSWORD_CAPTURE];
    m_hostname = matcher[HOSTNAME_CAPTURE];
    m_path = matcher[PATHNAME_CAPTURE];
    m_query = matcher[QUERY_CAPTURE];
    m_fragment = matcher[FRAGMENT_CAPTURE];
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
}

#endif
