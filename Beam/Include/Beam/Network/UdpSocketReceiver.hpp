#ifndef BEAM_UDP_SOCKET_RECEIVER_HPP
#define BEAM_UDP_SOCKET_RECEIVER_HPP
#include <cstdint>
#include <deque>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketOptions.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {

  /** Buffers UDP datagrams after the first receive or poll. */
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

      /** Returns whether a datagram is available. */
      bool poll() const;

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
      friend class MulticastSocketReader;
      friend class UdpSocketReader;
      struct Packet {
        SharedBuffer m_data;
        boost::asio::ip::udp::endpoint m_sender;
      };
      struct State {
        UdpSocketOptions m_options;
        std::shared_ptr<Details::UdpSocketEntry> m_socket;
        SharedBuffer m_buffer;
        boost::asio::ip::udp::endpoint m_sender;
        std::deque<Packet> m_packets;
        std::exception_ptr m_exception;
        ConditionVariable m_is_available;
        boost::asio::basic_waitable_timer<boost::chrono::steady_clock>
          m_deadline;
        std::uint64_t m_deadline_id;

        State(const UdpSocketOptions& options,
          std::shared_ptr<Details::UdpSocketEntry> socket);
      };
      std::shared_ptr<State> m_state;

      UdpSocketReceiver(const UdpSocketReceiver&) = delete;
      UdpSocketReceiver& operator =(const UdpSocketReceiver&) = delete;
      template<IsBuffer R>
      std::size_t receive(
        Out<R> destination, std::size_t size, IpAddress* address);
      static void start(const std::shared_ptr<State>& state);
      static void on_read(const std::shared_ptr<State>& state,
        const boost::system::error_code& error, std::size_t size);
      static void on_deadline(const std::shared_ptr<State>& state,
        std::uint64_t id, const boost::system::error_code& error);
  };

  inline UdpSocketReceiver::UdpSocketReceiver(const UdpSocketOptions& options,
      std::shared_ptr<Details::UdpSocketEntry> socket)
      : m_state(std::make_shared<State>(options, std::move(socket))) {
    auto error_code = boost::system::error_code();
    auto buffer_size = boost::asio::socket_base::receive_buffer_size(
      static_cast<int>(options.m_receive_buffer_size));
    m_state->m_socket->m_socket.set_option(buffer_size, error_code);
    if(error_code) {
      boost::throw_with_location(
        SocketException(error_code.value(), error_code.message()));
    }
  }

  inline UdpSocketReceiver::~UdpSocketReceiver() {
    m_state->m_socket->close();
    auto lock = std::lock_guard(m_state->m_socket->m_mutex);
    ++m_state->m_deadline_id;
    m_state->m_deadline.cancel();
  }

  inline bool UdpSocketReceiver::poll() const {
    auto lock = std::lock_guard(m_state->m_socket->m_mutex);
    if(!m_state->m_socket->m_is_open ||
        !m_state->m_socket->m_socket.is_open()) {
      return false;
    }
    auto is_available = !m_state->m_packets.empty();
    if(!is_available) {
      auto error = boost::system::error_code();
      is_available =
        m_state->m_socket->m_socket.available(error) != 0 && !error;
    }
    if(!m_state->m_socket->m_is_read_pending && !m_state->m_exception) {
      start(m_state);
    }
    return is_available;
  }

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<DatagramPacket<R>> packet, std::size_t size) {
    return receive(out(packet->get_data()), size, out(packet->get_address()));
  }

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<R> destination, std::size_t size, Out<IpAddress> address) {
    return receive(out(destination), size, address.get());
  }

  inline UdpSocketReceiver::State::State(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_options(options),
      m_socket(std::move(socket)),
      m_buffer(options.m_max_datagram_size),
      m_deadline(*m_socket->m_io_context),
      m_deadline_id(0) {}

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<R> destination, std::size_t size, IpAddress* address) {
    auto state = m_state;
    try {
      auto packet = [&] {
        auto lock = std::unique_lock(state->m_socket->m_mutex);
        if(!state->m_socket->m_is_open ||
            !state->m_socket->m_socket.is_open()) {
          boost::throw_with_location(EndOfFileException());
        }
        if(!state->m_socket->m_is_read_pending && !state->m_exception) {
          start(state);
        }
        auto has_timeout =
          state->m_options.m_timeout != boost::posix_time::pos_infin;
        if(state->m_packets.empty() && !state->m_exception && has_timeout) {
          auto id = ++state->m_deadline_id;
          state->m_deadline.expires_after(boost::chrono::microseconds(
            state->m_options.m_timeout.total_microseconds()));
          state->m_deadline.async_wait([=] (const auto& error) {
            on_deadline(state, id, error);
          });
        }
        while(state->m_packets.empty() && !state->m_exception &&
            state->m_socket->m_is_open) {
          state->m_is_available.wait(lock);
        }
        if(has_timeout) {
          ++state->m_deadline_id;
          state->m_deadline.cancel();
        }
        if(!state->m_socket->m_is_open ||
            !state->m_socket->m_socket.is_open()) {
          boost::throw_with_location(EndOfFileException());
        }
        if(state->m_packets.empty()) {
          std::rethrow_exception(std::exchange(state->m_exception, {}));
        }
        auto packet = std::move(state->m_packets.front());
        state->m_packets.pop_front();
        return packet;
      }();
      if(address) {
        *address = IpAddress(
          packet.m_sender.address().to_string(), packet.m_sender.port());
      }
      auto& data = packet.m_data;
      size = std::min(size, state->m_options.m_max_datagram_size);
      if constexpr(std::same_as<R, SharedBuffer>) {
        if(destination->get_size() == 0 && data.get_size() <= size) {
          size = data.get_size();
          *destination = std::move(data);
          return size;
        }
      }
      auto available = destination->grow(std::min(size, data.get_size()));
      if(available != 0) {
        destination->write(
          destination->get_size() - available, data.get_data(), available);
      }
      return available;
    } catch(const std::exception&) {
      throw_nested_with_location(EndOfFileException());
    }
  }

  inline void UdpSocketReceiver::start(const std::shared_ptr<State>& state) {
    state->m_socket->m_socket.async_receive_from(
      boost::asio::buffer(
        state->m_buffer.get_mutable_data(), state->m_buffer.get_size()),
      state->m_sender, [=] (const auto& error, auto size) {
        on_read(state, error, size);
      });
    state->m_socket->m_is_read_pending = true;
  }

  inline void UdpSocketReceiver::on_read(const std::shared_ptr<State>& state,
      const boost::system::error_code& error, std::size_t size) {
    auto lock = std::lock_guard(state->m_socket->m_mutex);
    state->m_socket->m_is_read_pending = false;
    ++state->m_deadline_id;
    try {
      if(error) {
        boost::throw_with_location(
          SocketException(error.value(), error.message()));
      }
      if(state->m_socket->m_is_open) {
        state->m_packets.emplace_back(
          SharedBuffer(state->m_buffer.get_data(), size), state->m_sender);
        start(state);
      }
    } catch(const std::exception&) {
      state->m_exception = std::current_exception();
    }
    if(state->m_packets.size() == 1 || state->m_exception ||
        !state->m_socket->m_is_open) {
      state->m_is_available.notify_all();
    }
    if(!state->m_socket->m_is_open) {
      state->m_socket->m_is_pending_condition.notify_all();
    }
  }

  inline void UdpSocketReceiver::on_deadline(
      const std::shared_ptr<State>& state, std::uint64_t id,
      const boost::system::error_code& error) {
    auto lock = std::lock_guard(state->m_socket->m_mutex);
    if(error == boost::asio::error::operation_aborted ||
        id != state->m_deadline_id || !state->m_socket->m_is_open) {
      return;
    }
    state->m_exception = std::make_exception_ptr(EndOfFileException());
    auto close_error = boost::system::error_code();
    state->m_socket->m_socket.close(close_error);
    state->m_is_available.notify_all();
  }
}

#endif
