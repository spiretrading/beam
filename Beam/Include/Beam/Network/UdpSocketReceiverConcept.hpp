#ifndef BEAM_UDP_SOCKET_RECEIVER_CONCEPT_HPP
#define BEAM_UDP_SOCKET_RECEIVER_CONCEPT_HPP
#include <concepts>
#include <memory>
#include <boost/asio/ip/udp.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/UdpSocketOptions.hpp"
#include "Beam/Pointers/Out.hpp"

namespace Beam::Details {
  template<typename S>
  struct SocketEntry;
}

namespace Beam {
  /** Concept satisfied by UDP datagram receivers. */
  template<typename T>
  concept IsUdpSocketReceiver = std::constructible_from<T,
    const UdpSocketOptions&,
    std::shared_ptr<Details::SocketEntry<boost::asio::ip::udp::socket>>> &&
    requires(T& receiver) {
      { std::as_const(receiver).poll() } -> std::same_as<bool>;
      { receiver.receive(
        out(std::declval<SharedBuffer&>()), std::size_t(0)) } ->
          std::same_as<std::size_t>;
      { receiver.receive(
        out(std::declval<DatagramPacket<SharedBuffer>&>())) } ->
          std::same_as<std::size_t>;
    };
}

#endif
