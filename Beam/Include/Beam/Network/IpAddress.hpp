#ifndef BEAM_IP_ADDRESS_HPP
#define BEAM_IP_ADDRESS_HPP
#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include "Beam/Network/Network.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/NotParser.hpp"
#include "Beam/Parsers/PlusParser.hpp"
#include "Beam/Parsers/Types.hpp"
#include "Beam/Utilities/AssertionException.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam::Network {

  /** Stores an IpAddress, consisting of a host and a port. */
  class IpAddress {
    public:

      /**
       * Converts an IP address represented as an int to a string.
       * @param ipAddress The IP address to convert, in integer form.
       * @return The string representation of the <i>ipAddress</i>.
       */
      static std::string IntToString(std::uint32_t ipAddress);

      /**
       * Converts an IP address represented as a string to an int.
       * @param ipAddress The IP address to convert, in string form.
       * @return The int representation of the <i>ipAddress</i>.
       */
      static std::uint32_t StringToInt(const std::string& ipAddress);

      /** Constructs an empty IpAddress. */
      IpAddress() = default;

      /**
       * Constructs an IpAddress.
       * @param host The host.
       * @param port The port.
       */
      IpAddress(std::string host, unsigned short port);

      /**
       * Returns <code>true</code> iff the host and port are identical.
       * @param rhs The right hand side of the comparison.
       * @return <code>true</code> iff the two IpAddresses are equal.
       */
      bool operator ==(const IpAddress& rhs) const;

      /**
       * Returns <code>true</code> iff the host or port are different.
       * @param rhs The right hand side of the comparison.
       * @return <code>true</code> iff the two IpAddresses are not equal.
       */
      bool operator !=(const IpAddress& rhs) const;

      /** Returns the host. */
      const std::string& GetHost() const;

      /** Returns the port. */
      unsigned short GetPort() const;

    private:
      std::string m_host;
      unsigned short m_port;
  };

  inline std::ostream& operator <<(std::ostream& out, const IpAddress& source) {
    return out << source.GetHost() << ':' << source.GetPort();
  }

  /** Parses an IpAddress. */
  inline auto IpAddressParser() {
    return Parsers::Convert(Parsers::PlusParser(Parsers::Not(':')) >> ':' >>
      Parsers::int_p,
      [] (const auto& source) {
        return IpAddress(std::get<0>(source),
          static_cast<unsigned short>(std::get<1>(source)));
      });
  }

  inline std::string IpAddress::IntToString(std::uint32_t ipAddress) {
    auto normalizedAddress = ToBigEndian(ipAddress);
    constexpr auto OCTET_COUNT = 4;
    auto octets = std::array<unsigned int, OCTET_COUNT>();
    for(auto i = 0; i < OCTET_COUNT; ++i) {
      octets[i] = (reinterpret_cast<const unsigned char*>(
        &normalizedAddress))[i];
    }
    constexpr auto BUFFER_SIZE = 16;
    auto buffer = std::array<char, BUFFER_SIZE>();
    std::sprintf(buffer.data(), "%-u.%-u.%-u.%-u", octets[0], octets[1],
      octets[2], octets[3]);
    return buffer.data();
  }

  inline std::uint32_t IpAddress::StringToInt(const std::string& ipAddress) {
    constexpr auto OCTET_COUNT = 4;
    auto octets = std::array<unsigned int, OCTET_COUNT>();
    octets.fill(0);
    auto result = std::sscanf(ipAddress.c_str(), "%u.%u.%u.%u", &octets[0],
      &octets[1], &octets[2], &octets[3]);
    BEAM_ASSERT(result == 4);
    auto intAddress = std::uint32_t(0);
    for(auto i = 0; i < OCTET_COUNT; ++i) {
      reinterpret_cast<unsigned char*>(&intAddress)[i] =
        static_cast<unsigned char>(octets[i]);
    }
    return FromBigEndian(intAddress);
  }

  inline IpAddress::IpAddress(std::string host, unsigned short port)
    : m_host(std::move(host)),
      m_port(port) {}

  inline bool IpAddress::operator ==(const IpAddress& rhs) const {
    return m_host == rhs.m_host && m_port == rhs.m_port;
  }

  inline bool IpAddress::operator !=(const IpAddress& rhs) const {
    return !(*this == rhs);
  }

  inline const std::string& IpAddress::GetHost() const {
    return m_host;
  }

  inline unsigned short IpAddress::GetPort() const {
    return m_port;
  }
}

namespace Beam::Parsers {
  template<>
  inline const auto default_parser<Beam::Network::IpAddress> =
    Beam::Network::IpAddressParser();
}

#endif
