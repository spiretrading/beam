module;
#include "Prelude.hpp"

export module Beam:UdpSocketSender;

import :DatagramPacket;
import :UdpSocketOptions;

export namespace Beam {

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
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      TaskRunner m_tasks;

      UdpSocketSender(const UdpSocketSender&) = delete;
      UdpSocketSender& operator =(const UdpSocketSender&) = delete;
  };

  inline UdpSocketSender::UdpSocketSender(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  template<IsConstBuffer R>
  void UdpSocketSender::send(const DatagramPacket<R>& packet) {
    auto write_result = Async<void>();
    m_socket->begin_write_operation();
    m_tasks.add([&] {
      auto destination_end = boost::asio::ip::udp::endpoint(
        boost::asio::ip::make_address(packet.get_address().get_host()),
        packet.get_address().get_port());
      m_socket->m_socket.async_send_to(boost::asio::buffer(
        packet.get_data().get_data(), packet.get_data().get_size()),
        destination_end, [&] (const auto& error, auto write_size) {
        if(error) {
          write_result .get_eval().set_exception(
            SocketException(error.value(), error.message()));
          return;
        }
        write_result .get_eval().set();
      });
    });
    try {
      write_result .get();
      m_socket->end_write_operation();
    } catch(const std::exception&) {
      m_socket->end_write_operation();
      std::throw_with_nested(EndOfFileException());
    }
  }
}

