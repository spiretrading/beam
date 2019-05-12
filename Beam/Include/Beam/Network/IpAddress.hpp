#ifndef BEAM_IPADDRESS_HPP
#define BEAM_IPADDRESS_HPP
#include <cstdint>
#include <string>
#include "Beam/Network/Network.hpp"
#include "Beam/Utilities/AssertionException.hpp"
#include "Beam/Utilities/Convert.hpp"
#include "Beam/Utilities/Endian.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace Network {

  /*! \class IpAddress
      \brief Stores an IpAddress, consisting of a host and a port.
   */
  class IpAddress {
    public:

      //! Converts an IP address represented as an int to a string.
      /*!
        \param ipAddress The IP address to convert, in integer form.
        \return The string representation of the <i>ipAddress</i>.
      */
      static std::string IntToString(std::uint32_t ipAddress);

      //! Converts an IP address represented as a string to an int.
      /*!
        \param ipAddress The IP address to convert, in string form.
        \return The int representation of the <i>ipAddress</i>.
      */
      static std::uint32_t StringToInt(const std::string& ipAddress);

      //! Constructs an empty IpAddress.
      IpAddress() = default;

      //! Constructs an IpAddress.
      /*!
        \param host The host.
        \param port The port.
      */
      IpAddress(std::string host, unsigned short port);

      //! Returns <code>true</code> iff the host and port are identical.
      /*!
        \param rhs The right hand side of the comparison.
        \return <code>true</code> iff the two IpAddresses are equal.
      */
      bool operator ==(const IpAddress& rhs) const;

      //! Returns <code>true</code> iff the host or port are different.
      /*!
        \param rhs The right hand side of the comparison.
        \return <code>true</code> iff the two IpAddresses are not equal.
      */
      bool operator !=(const IpAddress& rhs) const;

      //! Returns the host.
      const std::string& GetHost() const;

      //! Returns the port.
      unsigned short GetPort() const;

    private:
      std::string m_host;
      unsigned short m_port;
  };

  inline std::string IpAddress::IntToString(std::uint32_t ipAddress) {
    auto normalizedAddress = ToBigEndian(ipAddress);
    const auto OCTET_COUNT = 4;
    unsigned int octets[OCTET_COUNT];
    for(auto i = 0; i < OCTET_COUNT; ++i) {
      octets[i] = (reinterpret_cast<const unsigned char*>(
        &normalizedAddress))[i];
    }
    const auto BUFFER_SIZE = 16;
    char buffer[BUFFER_SIZE];
    std::sprintf(buffer, "%-u.%-u.%-u.%-u", octets[0], octets[1], octets[2],
      octets[3]);
    return buffer;
  }

  inline std::uint32_t IpAddress::StringToInt(const std::string& ipAddress) {
    const auto OCTET_COUNT = 4;
    unsigned int octets[OCTET_COUNT] = {0};
    auto result = std::sscanf(ipAddress.c_str(), "%u.%u.%u.%u", &octets[0],
      &octets[1], &octets[2], &octets[3]);
    BEAM_ASSERT(result == 4);
    auto intAddress = std::uint32_t{0};
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

  template<>
  inline std::string Convert(const Network::IpAddress& source) {
    return source.GetHost() + ":" + Convert<std::string>(source.GetPort());
  }

  template<>
  inline Network::IpAddress Convert(const std::string& source) {
    auto colonPosition = source.find(':');
    if(colonPosition == std::string::npos) {
      return {source, 0};
    }
    auto host = source.substr(0, colonPosition);
    auto port = Convert<unsigned short>(source.substr(colonPosition + 1));
    return {host, port};
  }
}

#endif
