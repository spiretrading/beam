#ifndef BEAM_COOKIE_HPP
#define BEAM_COOKIE_HPP
#include <ostream>
#include <sstream>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {
namespace Details {
  inline std::string FormatExpiration(const boost::posix_time::ptime& date) {
    std::stringstream ss;
    ss << date.date().day_of_week().as_short_string() << ", " <<
      date.date().day() << " " << date.date().month().as_short_string() <<
      " " << date.date().year() << " ";
    if(date.time_of_day().hours() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().hours() << ":";
    if(date.time_of_day().minutes() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().minutes() << ":";
    if(date.time_of_day().seconds() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().seconds() << " GMT";
    return ss.str();
  }
}

  /*! \class Cookie
      \brief Represents an HTTP cookie.
   */
  class Cookie {
    public:

      //! Constructs an empty Cookie.
      Cookie();

      //! Constructs a Cookie.
      /*!
        \param name The name of the Cookie.
        \param value The Cookie's value.
      */
      Cookie(std::string name, std::string value);

      //! Returns the name of the Cookie.
      const std::string& GetName() const;

      //! Returns the Cookie's value.
      const std::string& GetValue() const;

      //! Sets the Cookie's value.
      void SetValue(std::string value);

      //! Returns the domain.
      const std::string& GetDomain() const;

      //! Sets the domain.
      void SetDomain(std::string domain);

      //! Returns the path.
      const std::string& GetPath() const;

      //! Sets the path.
      void SetPath(std::string path);

      //! Returns the expiration.
      boost::posix_time::ptime GetExpiration() const;

      //! Sets the expiration.
      void SetExpiration(const boost::posix_time::ptime& expiration);

      //! Returns <code>true</code> iff this cookie is to only be sent over a
      //! secure protocol (such as HTTPS).
      bool IsSecure() const;

      //! Sets whether this cookie should only be sent over a secure protocol.
      void SetSecure(bool isSecure);

      //! Returns <code>true</code> iff this cookie is to only be sent over
      //! HTTP.
      bool IsHttpOnly() const;

      //! Sets whether this cookie should only be sent over HTTP.
      void SetHttpOnly(bool isHttpOnly);

    private:
      std::string m_name;
      std::string m_value;
      std::string m_domain;
      std::string m_path;
      boost::posix_time::ptime m_expiration;
      bool m_isSecure;
      bool m_isHttpOnly;
  };

  inline std::ostream& operator <<(std::ostream& sink, const Cookie& cookie) {
    sink << cookie.GetName() << "=" << cookie.GetValue();
    if(!cookie.GetDomain().empty()) {
      sink << "; Domain=" << cookie.GetDomain();
    }
    if(!cookie.GetPath().empty()) {
      sink << "; Path=" << cookie.GetPath();
    }
    if(!cookie.GetExpiration().is_not_a_date_time()) {
      sink << "; Expires=" << Details::FormatExpiration(cookie.GetExpiration());
    }
    if(cookie.IsSecure()) {
      sink << "; Secure";
    }
    if(cookie.IsHttpOnly()) {
      sink << "; HttpOnly";
    }
    return sink;
  }

  inline Cookie::Cookie()
      : m_isSecure{false},
        m_isHttpOnly{false},
        m_path{"/"} {}

  inline Cookie::Cookie(std::string name, std::string value)
      : m_name{std::move(name)},
        m_value{std::move(value)},
        m_isSecure{false},
        m_isHttpOnly{false},
        m_path{"/"} {}

  inline const std::string& Cookie::GetName() const {
    return m_name;
  }

  inline const std::string& Cookie::GetValue() const {
    return m_value;
  }

  inline void Cookie::SetValue(std::string value) {
    m_value = std::move(value);
  }

  inline const std::string& Cookie::GetDomain() const {
    return m_domain;
  }

  inline void Cookie::SetDomain(std::string domain) {
    m_domain = std::move(domain);
  }

  inline const std::string& Cookie::GetPath() const {
    return m_path;
  }

  inline void Cookie::SetPath(std::string path) {
    m_path = std::move(path);
  }

  inline boost::posix_time::ptime Cookie::GetExpiration() const {
    return m_expiration;
  }

  inline void Cookie::SetExpiration(
      const boost::posix_time::ptime& expiration) {
    m_expiration = expiration;
  }

  inline bool Cookie::IsSecure() const {
    return m_isSecure;
  }

  inline void Cookie::SetSecure(bool isSecure) {
    m_isSecure = isSecure;
  }

  inline bool Cookie::IsHttpOnly() const {
    return m_isHttpOnly;
  }

  inline void Cookie::SetHttpOnly(bool isHttpOnly) {
    m_isHttpOnly = isHttpOnly;
  }
}
}

#endif
