#ifndef BEAM_UDP_SOCKET_RECEIVER_HPP
#define BEAM_UDP_SOCKET_RECEIVER_HPP
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketOptions.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {

  /** Receives UDP datagrams. */
  class UdpSocketReceiver {
    public:

      /**
       * Constructs a UdpSocketReceiver.
       * @param options The options to apply to the receiver.
       * @param socket The socket to send the receive operations to.
       */
      UdpSocketReceiver(const UdpSocketOptions& options,
        std::shared_ptr<Details::UdpSocketEntry> socket);

      ~UdpSocketReceiver();

      /**
       * Receives a DatagramPacket.
       * @param packet The DatagramPacket that was received.
       * @param size The maximum size of the packet to receive.
       * @return The size of the received packet.
       */
      template<IsBuffer R>
      std::size_t receive(Out<DatagramPacket<R>> packet, std::size_t size = -1);

      /**
       * Receives a DatagramPacket.
       * @param destination Where to store the packet's data.
       * @param size The maximum size of the packet.
       * @param address The address of the packet's sender.
       * @return The size of the received packet.
       */
      template<IsBuffer R>
      std::size_t receive(
        Out<R> destination, std::size_t size, Out<IpAddress> address);

    private:
      mutable Mutex m_mutex;
      bool m_is_open;
      bool m_is_deadline_pending;
      ConditionVariable m_is_complete_condition;
      UdpSocketOptions m_options;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::asio::basic_waitable_timer<boost::chrono::steady_clock> m_deadline;

      UdpSocketReceiver(const UdpSocketReceiver&) = delete;
      UdpSocketReceiver& operator =(const UdpSocketReceiver&) = delete;
      void check_deadline(const boost::system::error_code& error);
  };

  inline UdpSocketReceiver::UdpSocketReceiver(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_is_open(true),
      m_is_deadline_pending(false),
      m_options(options),
      m_socket(std::move(socket)),
      m_deadline(*m_socket->m_io_context) {
    auto error_code = boost::system::error_code();
    auto buffer_size = boost::asio::socket_base::receive_buffer_size(
      static_cast<int>(m_options.m_receive_buffer_size));
    m_socket->m_socket.set_option(buffer_size, error_code);
    if(error_code) {
      boost::throw_with_location(
        SocketException(error_code.value(), error_code.message()));
    }
  }

  inline UdpSocketReceiver::~UdpSocketReceiver() {
    auto lock = boost::unique_lock(m_mutex);
    if(m_is_deadline_pending) {
      return;
    }
    m_is_open = false;
    m_deadline.cancel();
    while(m_is_deadline_pending) {
      m_is_complete_condition.wait(lock);
    }
  }

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<DatagramPacket<R>> packet, std::size_t size) {
    return receive(out(packet->get_data()), size, out(packet->get_address()));
  }

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<R> destination, std::size_t size, Out<IpAddress> address) {
    auto available_size =
      destination->grow(std::min(m_options.m_max_datagram_size, size));
    auto read_result = Async<std::size_t>();
    auto sender_end = boost::asio::ip::udp::endpoint();
    {
      auto lock = boost::lock_guard(m_socket->m_mutex);
      if(!m_socket->m_is_open) {
        boost::throw_with_location(EndOfFileException());
      }
      m_socket->m_is_read_pending = true;
      m_socket->m_socket.async_receive_from(
        boost::asio::buffer(get_mutable_suffix(*destination, available_size),
          available_size), sender_end,
        [&] (const auto& error, auto read_size) {
          if(error) {
            read_result.get_eval().set_exception(
              SocketException(error.value(), error.message()));
            return;
          }
          *address = IpAddress(
            sender_end.address().to_string(), sender_end.port());
          read_result.get_eval().set(read_size);
        });
    }
    auto has_timeout = m_options.m_timeout != boost::posix_time::pos_infin;
    if(has_timeout) {
      auto lock = boost::lock_guard(m_mutex);
      m_is_deadline_pending = true;
      m_deadline.expires_after(
        boost::chrono::microseconds(m_options.m_timeout.total_microseconds()));
      m_deadline.async_wait(
        std::bind_front(&UdpSocketReceiver::check_deadline, this));
    }
    try {
      auto result = read_result.get();
      if(has_timeout) {
        m_deadline.cancel();
      }
      m_socket->end_read_operation();
      destination->shrink(available_size - result);
      return result;
    } catch(const std::exception&) {
      m_socket->end_read_operation();
      std::throw_with_nested(EndOfFileException());
    }
  }

  inline void UdpSocketReceiver::check_deadline(
      const boost::system::error_code& error) {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_is_deadline_pending = false;
      if(!m_is_open) {
        m_is_complete_condition.notify_one();
        return;
      }
    }
    if(error != boost::asio::error::operation_aborted) {
      auto error_code = boost::system::error_code();
      {
        auto lock = boost::lock_guard(m_socket->m_mutex);
        m_socket->m_socket.close(error_code);
      }
    }
  }
}

#endif
