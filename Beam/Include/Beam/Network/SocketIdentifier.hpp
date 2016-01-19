#ifndef BEAM_SOCKETIDENTIFIER_HPP
#define BEAM_SOCKETIDENTIFIER_HPP
#include <cstdio>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/IpAddress.hpp"

namespace Beam {
namespace Network {

  /*! \class SocketIdentifier
      \brief Identifies a socket Channel using its IpAddress.
    */
  class SocketIdentifier {
    public:

      //! Constructs an empty SocketIdentifier.
      SocketIdentifier() = default;

      //! Constructs a SocketIdentifier.
      /*!
        \param address The IpAddress of the Channel.
      */
      SocketIdentifier(const IpAddress& address);

      //! Returns the IpAddress of the socket.
      const IpAddress& GetAddress() const;

      std::string ToString() const;

    private:
      IpAddress m_address;
  };

  inline SocketIdentifier::SocketIdentifier(const IpAddress& address)
    : m_address(address) {}

  inline const IpAddress& SocketIdentifier::GetAddress() const {
    return m_address;
  }

  inline std::string SocketIdentifier::ToString() const {
    char portBuffer[8];
    std::sprintf(portBuffer, ":%hu", m_address.GetPort());
    return m_address.GetHost() + portBuffer;
  }
}

  template<>
  struct ImplementsConcept<Network::SocketIdentifier, IO::ChannelIdentifier>
    : std::true_type {};
}

#endif
