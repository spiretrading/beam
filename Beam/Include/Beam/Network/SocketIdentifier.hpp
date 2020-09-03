#ifndef BEAM_SOCKET_IDENTIFIER_HPP
#define BEAM_SOCKET_IDENTIFIER_HPP
#include <ostream>
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/IpAddress.hpp"

namespace Beam {
namespace Network {

  /** Identifies a socket Channel using its IpAddress. */
  class SocketIdentifier {
    public:

      /** Constructs an empty SocketIdentifier. */
      SocketIdentifier() = default;

      /**
       * Constructs a SocketIdentifier.
       * @param address The IpAddress of the Channel.
       */
      SocketIdentifier(IpAddress address);

      /** Returns the IpAddress of the socket. */
      const IpAddress& GetAddress() const;

    private:
      IpAddress m_address;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const SocketIdentifier& identifier) {
    return out << identifier.GetAddress();
  }

  inline SocketIdentifier::SocketIdentifier(IpAddress address)
    : m_address(std::move(address)) {}

  inline const IpAddress& SocketIdentifier::GetAddress() const {
    return m_address;
  }
}

  template<>
  struct ImplementsConcept<Network::SocketIdentifier, IO::ChannelIdentifier>
    : std::true_type {};
}

#endif
