#ifndef BEAM_COOKIE_HPP
#define BEAM_COOKIE_HPP
#include <ostream>
#include <string>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

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

      //! Returns the domain.
      const std::string& GetDomain() const;

      //! Sets the domain.
      void SetDomain(std::string domain);

      //! Returns the path.
      const std::string& GetPath() const;

      //! Sets the path.
      void SetPath(std::string path);

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
