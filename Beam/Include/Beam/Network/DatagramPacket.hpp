#ifndef BEAM_DATAGRAM_PACKET_HPP
#define BEAM_DATAGRAM_PACKET_HPP
#include <ostream>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Stores a single datagram packet. */
  template<IsConstBuffer B>
  class DatagramPacket {
    public:

      /** The type of Buffer stored. */
      using Buffer = B;

      /** Constructs an empty DatagramPacket. */
      DatagramPacket() = default;

      /**
       * Constructs a DatagramPacket.
       * @param data The data stored by the packet.
       * @param address The address that this packet was received from.
       */
      template<typename BF>
      DatagramPacket(BF&& data, IpAddress address);

      /** Returns the data stored by this packet. */
      const Buffer& get_data() const;

      /** Returns the data stored by this packet. */
      Buffer& get_data() requires IsBuffer<B>;

      /** Returns the address that this packet was received from. */
      const IpAddress& get_address() const;

      /** Returns the address that this packet was received from. */
      IpAddress& get_address();

    private:
      Buffer m_data;
      IpAddress m_address;
  };

  template<typename B>
  DatagramPacket(B&&, IpAddress) -> DatagramPacket<std::remove_cvref_t<B>>;

  template<IsBuffer B>
  std::ostream& operator <<(
      std::ostream& out, const DatagramPacket<B>& packet) {
    return out << packet.get_address() << ':' << packet.get_data();
  }

  template<IsConstBuffer B>
  template<typename BF>
  DatagramPacket<B>::DatagramPacket(BF&& data, IpAddress address)
    : m_data(std::forward<BF>(data)),
      m_address(std::move(address)) {}

  template<IsConstBuffer B>
  const typename DatagramPacket<B>::Buffer&
      DatagramPacket<B>::get_data() const {
    return m_data;
  }

  template<IsConstBuffer B>
  typename DatagramPacket<B>::Buffer& DatagramPacket<B>::get_data() requires
      IsBuffer<B> {
    return m_data;
  }

  template<IsConstBuffer B>
  const IpAddress& DatagramPacket<B>::get_address() const {
    return m_address;
  }

  template<IsConstBuffer B>
  IpAddress& DatagramPacket<B>::get_address() {
    return m_address;
  }
}

#endif
