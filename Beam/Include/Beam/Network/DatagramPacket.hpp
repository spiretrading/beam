#ifndef BEAM_DATAGRAM_PACKET_HPP
#define BEAM_DATAGRAM_PACKET_HPP
#include <ostream>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Network/IpAddress.hpp"

namespace Beam::Network {

  /** Stores a single datagram packet. */
  template<typename B>
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
      const Buffer& GetData() const;

      /** Returns the data stored by this packet. */
      Buffer& GetData();

      /** Returns the address that this packet was received from. */
      const IpAddress& GetAddress() const;

      /** Returns the address that this packet was received from. */
      IpAddress& GetAddress();

    private:
      Buffer m_data;
      IpAddress m_address;
  };

  template<typename Buffer>
  std::ostream& operator <<(std::ostream& out,
      const DatagramPacket<Buffer>& packet) {
    return out << packet.GetAddress() << ':' << packet.GetData();
  }

  template<typename B>
  template<typename BF>
  DatagramPacket<B>::DatagramPacket(BF&& data, IpAddress address)
    : m_data(std::forward<BF>(data)),
      m_address(std::move(address)) {}

  template<typename B>
  const typename DatagramPacket<B>::Buffer& DatagramPacket<B>::GetData() const {
    return m_data;
  }

  template<typename B>
  typename DatagramPacket<B>::Buffer& DatagramPacket<B>::GetData() {
    return m_data;
  }

  template<typename B>
  const IpAddress& DatagramPacket<B>::GetAddress() const {
    return m_address;
  }

  template<typename B>
  IpAddress& DatagramPacket<B>::GetAddress() {
    return m_address;
  }
}

#endif
