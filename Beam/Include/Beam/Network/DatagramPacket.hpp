#ifndef BEAM_DATAGRAMPACKET_HPP
#define BEAM_DATAGRAMPACKET_HPP
#include "Beam/IO/Buffer.hpp"
#include "Beam/Network/IpAddress.hpp"

namespace Beam {
namespace Network {

  /*! \class DatagramPacket
      \brief Stores a single datagram packet.
   */
  template<typename BufferType>
  class DatagramPacket {
    public:

      //! The type of Buffer stored.
      using Buffer = BufferType;

      //! Constructs a DatagramPacket.
      DatagramPacket() = default;

      //! Constructs a DatagramPacket.
      /*!
        \param data The data stored by the packet.
        \param address The address that this packet was received from.
      */
      template<typename BufferForward, typename IpAddressForward>
      DatagramPacket(BufferForward&& data, IpAddressForward&& address);

      //! Returns the data stored by this packet.
      const Buffer& GetData() const;

      //! Returns the data stored by this packet.
      Buffer& GetData();

      //! Returns the address that this packet was received from.
      const IpAddress& GetAddress() const;

      //! Returns the address that this packet was received from.
      IpAddress& GetAddress();

      //! Swaps this packet with another.
      /*!
        \param packet The Datagram packet to swap with.
      */
      void Swap(DatagramPacket& packet);

    private:
      Buffer m_data;
      IpAddress m_address;
  };

  //! Exchanges the contents of two DatagramPackets.
  /*!
    \param a One of the two DatagramPackets to swap.
    \param b One of the two DatagramPackets to swap.
  */
  template<typename BufferType>
  void swap(DatagramPacket<BufferType>& a, DatagramPacket<BufferType>& b) {
    a.Swap(b);
  }

  template<typename BufferType>
  template<typename BufferForward, typename IpAddressForward>
  DatagramPacket<BufferType>::DatagramPacket(BufferForward&& data,
      IpAddressForward&& address)
      : m_data(std::forward<BufferForward>(data)),
        m_address(std::forward<IpAddressForward>(address)) {}

  template<typename BufferType>
  const BufferType& DatagramPacket<BufferType>::GetData() const {
    return m_data;
  }

  template<typename BufferType>
  BufferType& DatagramPacket<BufferType>::GetData() {
    return m_data;
  }

  template<typename BufferType>
  const IpAddress& DatagramPacket<BufferType>::GetAddress() const {
    return m_address;
  }

  template<typename BufferType>
  IpAddress& DatagramPacket<BufferType>::GetAddress() {
    return m_address;
  }

  template<typename BufferType>
  void DatagramPacket<BufferType>::Swap(DatagramPacket& packet) {
    m_data.Swap(packet.m_data);
    m_address.Swap(packet.m_address);
  }
}
}

#endif
