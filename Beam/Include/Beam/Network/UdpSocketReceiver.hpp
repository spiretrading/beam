#ifndef BEAM_UDP_SOCKET_RECEIVER_HPP
#define BEAM_UDP_SOCKET_RECEIVER_HPP
#include <cstdint>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/UdpSocketReceiverConcept.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {

  /** Receives UDP datagrams on demand. */
  class UdpSocketReceiver {
    public:

      /**
       * Constructs a UdpSocketReceiver.
       * @param options The options to apply to the receiver.
       * @param socket The socket to receive from.
       */
      UdpSocketReceiver(const UdpSocketOptions& options,
        std::shared_ptr<Details::UdpSocketEntry> socket);

      ~UdpSocketReceiver();

      /** Returns whether a datagram is available. */
      bool poll() const;

      /** Receives a datagram and its sender address. */
      template<IsBuffer R>
      std::size_t receive(Out<DatagramPacket<R>> packet);

      /**
       * Receives a datagram and its sender address.
       * @param packet Where to store the datagram.
       * @param size The maximum number of bytes to receive.
       */
      template<IsBuffer R>
      std::size_t receive(Out<DatagramPacket<R>> packet, std::size_t size);

      /**
       * Receives a datagram.
       * @param destination Where to append the datagram's data.
       * @param size The maximum number of bytes to receive.
       * @param address Where to store the sender address.
       */
      template<IsBuffer R>
      std::size_t receive(
        Out<R> destination, std::size_t size, Out<IpAddress> address);

      /** Receives a datagram without its sender address. */
      template<IsBuffer R>
      std::size_t receive(Out<R> destination, std::size_t size);

    private:
      struct State {
        UdpSocketOptions m_options;
        std::shared_ptr<Details::UdpSocketEntry> m_socket;
        boost::asio::basic_waitable_timer<boost::chrono::steady_clock>
          m_deadline;
        std::uint64_t m_deadline_id;
        boost::asio::cancellation_signal m_cancellation;

        State(const UdpSocketOptions& options,
          std::shared_ptr<Details::UdpSocketEntry> socket);
      };
      std::shared_ptr<State> m_state;

      UdpSocketReceiver(const UdpSocketReceiver&) = delete;
      UdpSocketReceiver& operator =(const UdpSocketReceiver&) = delete;
      template<IsBuffer R>
      std::size_t receive(
        Out<R> destination, std::size_t size, IpAddress* address);
      static void on_deadline(const std::shared_ptr<State>& state,
        std::uint64_t id, const boost::system::error_code& error);
  };

  inline UdpSocketReceiver::UdpSocketReceiver(const UdpSocketOptions& options,
      std::shared_ptr<Details::UdpSocketEntry> socket)
      : m_state(std::make_shared<State>(options, std::move(socket))) {
    auto error = boost::system::error_code();
    m_state->m_socket->m_socket.set_option(
      boost::asio::socket_base::receive_buffer_size(
        static_cast<int>(options.m_receive_buffer_size)), error);
    if(error) {
      boost::throw_with_location(
        SocketException(error.value(), error.message()));
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
    if(!m_state->m_socket->m_is_open) {
      return false;
    }
    auto error = boost::system::error_code();
    return m_state->m_socket->m_socket.available(error) != 0 && !error;
  }

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(Out<DatagramPacket<R>> packet) {
    return receive(out(packet), std::size_t(-1));
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

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<R> destination, std::size_t size) {
    return receive(out(destination), size, nullptr);
  }

  inline UdpSocketReceiver::State::State(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_options(options),
      m_socket(std::move(socket)),
      m_deadline(*m_socket->m_io_context),
      m_deadline_id(0) {}

  template<IsBuffer R>
  std::size_t UdpSocketReceiver::receive(
      Out<R> destination, std::size_t size, IpAddress* address) {
    auto state = m_state;
    auto available = destination->grow(
      std::min(size, state->m_options.m_max_datagram_size));
    auto result = Async<std::size_t>();
    auto sender = boost::asio::ip::udp::endpoint();
    auto is_read_pending = false;
    auto has_timeout =
      state->m_options.m_timeout != boost::posix_time::pos_infin;
    auto count = std::size_t(0);
    try {
      {
        auto lock = std::lock_guard(state->m_socket->m_mutex);
        if(!state->m_socket->m_is_open ||
            !state->m_socket->m_socket.is_open()) {
          boost::throw_with_location(EndOfFileException());
        }
        auto cancellation = boost::asio::cancellation_slot();
        if(has_timeout) {
          cancellation = state->m_cancellation.slot();
          auto id = ++state->m_deadline_id;
          state->m_deadline.expires_after(boost::chrono::microseconds(
            state->m_options.m_timeout.total_microseconds()));
          state->m_deadline.async_wait([=] (const auto& error) {
            on_deadline(state, id, error);
          });
        }
        state->m_socket->m_socket.async_receive_from(boost::asio::buffer(
          get_mutable_suffix(*destination, available), available), sender,
          boost::asio::bind_cancellation_slot(cancellation,
            [&, state] (const auto& error, auto size) {
              auto lock = std::lock_guard(state->m_socket->m_mutex);
              state->m_cancellation.slot().clear();
              ++state->m_deadline_id;
              if(error) {
                result.get_eval().set_exception(
                  SocketException(error.value(), error.message()));
              } else {
                result.get_eval().set(size);
              }
            }));
        state->m_socket->m_is_read_pending = true;
        is_read_pending = true;
      }
      count = result.get();
    } catch(const std::exception&) {
      {
        auto lock = std::lock_guard(state->m_socket->m_mutex);
        ++state->m_deadline_id;
        if(has_timeout) {
          state->m_deadline.cancel();
        }
      }
      if(is_read_pending) {
        state->m_socket->end_read_operation();
      }
      destination->shrink(available);
      throw_nested_with_location(EndOfFileException());
    }
    {
      auto lock = std::lock_guard(state->m_socket->m_mutex);
      ++state->m_deadline_id;
      if(has_timeout) {
        state->m_deadline.cancel();
      }
    }
    state->m_socket->end_read_operation();
    destination->shrink(available - count);
    if(address) {
      *address = IpAddress(sender.address().to_string(), sender.port());
    }
    return count;
  }

  inline void UdpSocketReceiver::on_deadline(
      const std::shared_ptr<State>& state, std::uint64_t id,
      const boost::system::error_code& error) {
    auto lock = std::lock_guard(state->m_socket->m_mutex);
    if(error == boost::asio::error::operation_aborted ||
        id != state->m_deadline_id || !state->m_socket->m_is_open) {
      return;
    }
    state->m_cancellation.emit(boost::asio::cancellation_type::total);
  }
}

#endif
