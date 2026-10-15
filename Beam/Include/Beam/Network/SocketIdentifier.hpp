#ifndef BEAM_SOCKET_IDENTIFIER_HPP
#define BEAM_SOCKET_IDENTIFIER_HPP
#include "Beam/IO/ChannelIdentifier.hpp"
#include "Beam/Network/IpAddress.hpp"

namespace Beam {

  /** Identifies a socket Channel using its IpAddress. */
  class SocketIdentifier {
    public:

      /** Constructs an empty SocketIdentifier. */
      SocketIdentifier() = default;

      /**
       * Constructs a SocketIdentifier.
       * @param address The IpAddress of the Channel.
       */
      SocketIdentifier(IpAddress address) noexcept;

      /** Returns the IpAddress of the socket. */
      const IpAddress& get_address() const;

    private:
      IpAddress m_address;
  };

  inline SocketIdentifier::SocketIdentifier(IpAddress address) noexcept
    : m_address(std::move(address)) {}

  inline const IpAddress& SocketIdentifier::get_address() const {
    return m_address;
  }
}

template<>
class std::formatter<Beam::SocketIdentifier> :
    public std::formatter<Beam::IpAddress> {
  public:
    auto format(const Beam::SocketIdentifier& value, auto& context) const {
      return std::formatter<Beam::IpAddress>::format(
        value.get_address(), context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& out, const SocketIdentifier& identifier) {
    return out << std::format("{}", identifier);
  }
}

#endif
