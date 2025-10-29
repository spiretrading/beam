#ifndef BEAM_IP_ADDRESS_HPP
#define BEAM_IP_ADDRESS_HPP
#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <boost/endian.hpp>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Utilities/AssertionException.hpp"

namespace Beam {

  /** Stores an IpAddress, consisting of a host and a port. */
  class IpAddress {
    public:

      /**
       * Converts an IP address represented as an int to a string.
       * @param ipAddress The IP address to convert, in integer form.
       * @return The string representation of the <i>ipAddress</i>.
       */
      static std::string int_to_string(std::uint32_t address);

      /**
       * Converts an IP address represented as a string to an int.
       * @param ipAddress The IP address to convert, in string form.
       * @return The int representation of the <i>ipAddress</i>.
       */
      static std::uint32_t string_to_int(std::string_view address);

      /** Constructs an empty IpAddress. */
      IpAddress() = default;

      /**
       * Constructs an IpAddress.
       * @param host The host.
       * @param port The port.
       */
      IpAddress(std::string host, unsigned short port) noexcept;

      /** Returns the host. */
      const std::string& get_host() const;

      /** Returns the port. */
      unsigned short get_port() const;

      bool operator ==(const IpAddress&) const = default;

    private:
      std::string m_host;
      unsigned short m_port;
  };

  /** Parses an IpAddress. */
  inline auto ip_address_parser() {
    return convert(+(!(symbol(":"))) >> ':' >> int_p, [] (const auto& source) {
      return IpAddress(std::get<0>(source),
        static_cast<unsigned short>(std::get<1>(source)));
    });
  }

  /** A parser that matches an IpAddress. */
  inline const auto ip_address_p = ip_address_parser();

  template<>
  inline const auto default_parser<IpAddress> = ip_address_p;

  inline std::ostream& operator <<(std::ostream& out, const IpAddress& source) {
    return out << source.get_host() << ':' << source.get_port();
  }

  inline std::string IpAddress::int_to_string(std::uint32_t address) {
    address = boost::endian::native_to_big(address);
    const auto OCTET_COUNT = 4;
    auto octets = std::array<unsigned int, OCTET_COUNT>();
    for(auto i = 0; i < OCTET_COUNT; ++i) {
      octets[i] = (reinterpret_cast<const unsigned char*>(&address))[i];
    }
    const auto BUFFER_SIZE = 16;
    auto buffer = std::array<char, BUFFER_SIZE>();
    std::sprintf(buffer.data(), "%-u.%-u.%-u.%-u", octets[0], octets[1],
      octets[2], octets[3]);
    return buffer.data();
  }

  inline std::uint32_t IpAddress::string_to_int(std::string_view address) {
    const auto OCTET_COUNT = 4;
    auto octets = std::array<unsigned int, OCTET_COUNT>();
    octets.fill(0);
    auto result = std::sscanf(address.data(), "%u.%u.%u.%u", &octets[0],
      &octets[1], &octets[2], &octets[3]);
    BEAM_ASSERT(result == 4);
    auto int_address = std::uint32_t(0);
    for(auto i = 0; i < OCTET_COUNT; ++i) {
      reinterpret_cast<unsigned char*>(&int_address)[i] =
        static_cast<unsigned char>(octets[i]);
    }
    return boost::endian::native_to_big(int_address);
  }

  inline IpAddress::IpAddress(std::string host, unsigned short port) noexcept
    : m_host(std::move(host)),
      m_port(port) {}

  inline const std::string& IpAddress::get_host() const {
    return m_host;
  }

  inline unsigned short IpAddress::get_port() const {
    return m_port;
  }
}

#endif
