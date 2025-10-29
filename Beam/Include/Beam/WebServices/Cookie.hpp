#ifndef BEAM_COOKIE_HPP
#define BEAM_COOKIE_HPP
#include <ostream>
#include <sstream>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Beam {
namespace Details {
  inline std::string format_expiration(boost::posix_time::ptime date) {
    auto ss = std::stringstream();
    ss << date.date().day_of_week().as_short_string() << ", " <<
      date.date().day() << ' ' << date.date().month().as_short_string() <<
      ' ' << date.date().year() << ' ';
    if(date.time_of_day().hours() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().hours() << ':';
    if(date.time_of_day().minutes() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().minutes() << ':';
    if(date.time_of_day().seconds() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().seconds() << " GMT";
    return ss.str();
  }
}

  /** Represents an HTTP cookie. */
  class Cookie {
    public:

      /** Constructs an empty Cookie. */
      Cookie() noexcept;

      /**
       * Constructs a Cookie.
       * @param name The name of the Cookie.
       * @param value The Cookie's value.
       */
      Cookie(std::string name, std::string value) noexcept;

      /** Returns the name of the Cookie. */
      const std::string& get_name() const;

      /** Returns the Cookie's value. */
      const std::string& get_value() const;

      /** Sets the Cookie's value. */
      void set_value(const std::string& value);

      /** Returns the domain. */
      const std::string& get_domain() const;

      /** Sets the domain. */
      void set_domain(const std::string& domain);

      /** Returns the path. */
      const std::string& get_path() const;

      /** Sets the path. */
      void set_path(const std::string& path);

      /** Returns the expiration. */
      boost::posix_time::ptime get_expiration() const;

      /** Sets the expiration. */
      void set_expiration(boost::posix_time::ptime expiration);

      /**
       * Returns <code>true</code> iff this cookie is to only be sent over a
       * secure protocol (such as HTTPS).
       */
      bool is_secure() const;

      /**
       * Sets whether this cookie should only be sent over a secure protocol.
       */
      void set_secure(bool is_secure);

      /**
       * Returns <code>true</code> iff this cookie is to only be sent over
       * HTTP.
       */
      bool is_http_only() const;

      /** Sets whether this cookie should only be sent over HTTP. */
      void set_http_only(bool is_http_only);

    private:
      std::string m_name;
      std::string m_value;
      std::string m_domain;
      std::string m_path;
      boost::posix_time::ptime m_expiration;
      bool m_is_secure;
      bool m_is_http_only;
  };

  inline std::ostream& operator <<(std::ostream& sink, const Cookie& cookie) {
    sink << cookie.get_name() << '=' << cookie.get_value();
    if(!cookie.get_domain().empty()) {
      sink << "; Domain=" << cookie.get_domain();
    }
    if(!cookie.get_path().empty()) {
      sink << "; Path=" << cookie.get_path();
    }
    if(!cookie.get_expiration().is_not_a_date_time()) {
      sink << "; Expires=" <<
        Details::format_expiration(cookie.get_expiration());
    }
    if(cookie.is_secure()) {
      sink << "; Secure";
    }
    if(cookie.is_http_only()) {
      sink << "; HttpOnly";
    }
    return sink;
  }

  inline Cookie::Cookie() noexcept
    : m_is_secure(false),
      m_is_http_only(false),
      m_path("/") {}

  inline Cookie::Cookie(std::string name, std::string value) noexcept
    : m_name(std::move(name)),
      m_value(std::move(value)),
      m_is_secure(false),
      m_is_http_only(false),
      m_path("/") {}

  inline const std::string& Cookie::get_name() const {
    return m_name;
  }

  inline const std::string& Cookie::get_value() const {
    return m_value;
  }

  inline void Cookie::set_value(const std::string& value) {
    m_value = value;
  }

  inline const std::string& Cookie::get_domain() const {
    return m_domain;
  }

  inline void Cookie::set_domain(const std::string& domain) {
    m_domain = domain;
  }

  inline const std::string& Cookie::get_path() const {
    return m_path;
  }

  inline void Cookie::set_path(const std::string& path) {
    m_path = path;
  }

  inline boost::posix_time::ptime Cookie::get_expiration() const {
    return m_expiration;
  }

  inline void Cookie::set_expiration(boost::posix_time::ptime expiration) {
    m_expiration = expiration;
  }

  inline bool Cookie::is_secure() const {
    return m_is_secure;
  }

  inline void Cookie::set_secure(bool is_secure) {
    m_is_secure = is_secure;
  }

  inline bool Cookie::is_http_only() const {
    return m_is_http_only;
  }

  inline void Cookie::set_http_only(bool is_http_only) {
    m_is_http_only = is_http_only;
  }
}

#endif
