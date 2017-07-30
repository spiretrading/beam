#ifndef BEAM_EMAILADDRESS_HPP
#define BEAM_EMAILADDRESS_HPP
#include <ostream>
#include <string>
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class EmailAddress
      \brief Represents an email address.
   */
  class EmailAddress {
    public:

      //! Constructs an email address.
      /*!
        \param address The email address.
       */
      EmailAddress(std::string address);

      //! Constructs an email address.
      /*!
        \param address The email address.
        \param displayName The address's display name.
      */
      EmailAddress(std::string address, std::string displayName);

      //! Returns the proper address.
      const std::string& GetAddress() const;

      //! Returns the display name.
      const std::string& GetDisplayName() const;

      //! Returns the address's user portion (that which precedes the @ sign).
      std::string GetUser() const;

      //! Returns the address's domain (that which follows the @ sign).
      std::string GetDomain() const;

    private:
      std::string m_address;
      std::string m_displayName;
  };

  inline std::ostream& operator <<(std::ostream& sink,
      const EmailAddress& address) {
    if(!address.GetDisplayName().empty()) {
      sink << address.GetDisplayName() << " <" << address.GetAddress() << ">";
    } else {
      sink << address.GetAddress();
    }
    return sink;
  }

  inline EmailAddress::EmailAddress(std::string address)
      : m_address{std::move(address)} {}

  inline EmailAddress::EmailAddress(std::string address,
      std::string displayName)
      : m_address{std::move(address)},
        m_displayName{std::move(displayName)} {}

  inline const std::string& EmailAddress::GetAddress() const {
    return m_address;
  }

  inline const std::string& EmailAddress::GetDisplayName() const {
    return m_displayName;
  }

  inline std::string EmailAddress::GetUser() const {
    auto separator = m_address.find('@');
    if(separator == std::string::npos) {
      return m_address;
    }
    return m_address.substr(0, separator);
  }

  inline std::string EmailAddress::GetDomain() const {
    auto separator = m_address.find('@');
    if(separator == std::string::npos) {
      return {};
    }
    return m_address.substr(separator + 1);
  }
}
}

#endif
