#ifndef BEAM_EMAIL_ADDRESS_HPP
#define BEAM_EMAIL_ADDRESS_HPP
#include <ostream>
#include <string>

namespace Beam {

  /** Represents an email address. */
  class EmailAddress {
    public:

      /**
       * Constructs an email address.
       * @param address The email address.
       */
      EmailAddress(std::string address) noexcept;

      /**
       * Constructs an email address.
       * @param address The email address.
       */
      EmailAddress(const char* address);

      /**
       * Constructs an email address.
       * @param address The email address.
       * @param display_name The address's display name.
       */
      EmailAddress(std::string address, std::string display_name) noexcept;

      /** Returns the proper address. */
      const std::string& get_address() const;

      /** Returns the display name. */
      const std::string& get_display_name() const;

      /**
       * Returns the address's user portion (that which precedes the @ sign).
       */
      std::string get_user() const;

      /** Returns the address's domain (that which follows the @ sign). */
      std::string get_domain() const;

    private:
      std::string m_address;
      std::string m_display_name;
  };

  inline std::ostream& operator <<(
      std::ostream& sink, const EmailAddress& address) {
    if(!address.get_display_name().empty()) {
      sink << address.get_display_name() << " <" << address.get_address() <<
        '>';
    } else {
      sink << address.get_address();
    }
    return sink;
  }

  inline EmailAddress::EmailAddress(std::string address) noexcept
    : m_address(std::move(address)) {}

  inline EmailAddress::EmailAddress(const char* address)
    : EmailAddress(std::string(address)) {}

  inline EmailAddress::EmailAddress(
    std::string address, std::string display_name) noexcept
    : m_address(std::move(address)),
      m_display_name(std::move(display_name)) {}

  inline const std::string& EmailAddress::get_address() const {
    return m_address;
  }

  inline const std::string& EmailAddress::get_display_name() const {
    return m_display_name;
  }

  inline std::string EmailAddress::get_user() const {
    auto separator = m_address.find('@');
    if(separator == std::string::npos) {
      return m_address;
    }
    return m_address.substr(0, separator);
  }

  inline std::string EmailAddress::get_domain() const {
    auto separator = m_address.find('@');
    if(separator == std::string::npos) {
      return {};
    }
    return m_address.substr(separator + 1);
  }
}

#endif
