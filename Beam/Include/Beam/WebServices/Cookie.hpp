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
      Cookie() = default;

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

    private:
      std::string m_name;
      std::string m_value;
  };

  inline std::ostream& operator <<(std::ostream& sink, const Cookie& cookie) {
    return (sink << cookie.GetName() << "=" << cookie.GetValue());
  }

  inline Cookie::Cookie(std::string name, std::string value)
      : m_name{std::move(name)},
        m_value{std::move(value)} {}

  inline const std::string& Cookie::GetName() const {
    return m_name;
  }

  inline const std::string& Cookie::GetValue() const {
    return m_value;
  }
}
}

#endif
