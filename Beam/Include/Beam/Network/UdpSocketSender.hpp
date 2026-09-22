#ifndef BEAM_UDP_SOCKET_SENDER_HPP
#define BEAM_UDP_SOCKET_SENDER_HPP
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {

  /** Sends datagrams via a UDP socket. */
  class UdpSocketSender {
    public:

      /**
       * Constructs a UdpSocketSender.
       * @param socket The socket to send the data to.
       */
      UdpSocketSender(const UdpSocketOptions& options,
        std::shared_ptr<Details::UdpSocketEntry> socket);

      /**
       * Sends a DatagramPacket.
       * @param packet The DatagramPacket to send.
       */
      template<IsConstBuffer R>
      void send(const DatagramPacket<R>& packet);

    private:
      template<IsUdpSocketReceiver R>
      friend class BasicMulticastSocketWriter;
      template<IsUdpSocketReceiver R>
      friend class BasicUdpSocketWriter;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;

      UdpSocketSender(const UdpSocketSender&) = delete;
      UdpSocketSender& operator =(const UdpSocketSender&) = delete;
      template<IsConstBuffer R>
      void send(
        const R& data, const boost::asio::ip::udp::endpoint& destination);
  };

  inline UdpSocketSender::UdpSocketSender(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  template<IsConstBuffer R>
  void UdpSocketSender::send(const DatagramPacket<R>& packet) {
    auto destination = boost::asio::ip::udp::endpoint(
      boost::asio::ip::make_address(packet.get_address().get_host()),
      packet.get_address().get_port());
    send(packet.get_data(), destination);
  }

  template<IsConstBuffer R>
  void UdpSocketSender::send(
      const R& data, const boost::asio::ip::udp::endpoint& destination) {
    auto write_result = Async<void>();
    try {
      auto lock = std::lock_guard(m_socket->m_mutex);
      if(!m_socket->m_is_open) {
        boost::throw_with_location(EndOfFileException());
      }
      m_socket->m_socket.async_send_to(
        boost::asio::buffer(data.get_data(), data.get_size()), destination,
        [&] (const auto& error, auto write_size) {
          if(error) {
            write_result.get_eval().set_exception(
              SocketException(error.value(), error.message()));
            return;
          }
          write_result.get_eval().set();
        });
      ++m_socket->m_pending_writes;
    } catch(const std::exception&) {
      throw_nested_with_location(EndOfFileException());
    }
    try {
      write_result.get();
      m_socket->end_write_operation();
    } catch(const std::exception&) {
      m_socket->end_write_operation();
      throw_nested_with_location(EndOfFileException());
    }
  }
}

#endif
