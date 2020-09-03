#ifndef BEAM_UDP_SOCKET_SENDER_HPP
#define BEAM_UDP_SOCKET_SENDER_HPP
#include <boost/asio/ip/udp.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/UdpSocketOptions.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam::Network {

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
      template<typename Buffer>
      void Send(const DatagramPacket<Buffer>& packet);

      /**
       * Sends a DatagramPacket.
       * @param data A pointer to the packet's data.
       * @param size The size of the packet.
       * @param destination The packet's destination.
       */
      void Send(const void* data, std::size_t size,
        const IpAddress& destination);

    private:
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      Threading::TaskRunner m_tasks;

      UdpSocketSender(const UdpSocketSender&) = delete;
      UdpSocketSender& operator =(const UdpSocketSender&) = delete;
  };

  inline UdpSocketSender::UdpSocketSender(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  template<typename Buffer>
  void UdpSocketSender::Send(const DatagramPacket<Buffer>& packet) {
    Send(packet.GetData().GetData(), packet.GetData().GetSize(),
      packet.GetAddress());
  }

  inline void UdpSocketSender::Send(const void* data, std::size_t size,
      const IpAddress& destination) {
    auto writeResult = Routines::Async<void>();
    m_socket->BeginWriteOperation();
    m_tasks.Add([&] {
      auto destinationEndpoint = boost::asio::ip::udp::endpoint(
        boost::asio::ip::address::from_string(destination.GetHost()),
        destination.GetPort());
      m_socket->m_socket.async_send_to(boost::asio::buffer(data, size),
          destinationEndpoint, [&] (const auto& error, auto writeSize) {
        if(error) {
          if(Details::IsEndOfFile(error)) {
            writeResult.GetEval().SetException(IO::EndOfFileException());
            return;
          }
          writeResult.GetEval().SetException(SocketException(error.value(),
            error.message()));
          return;
        }
        writeResult.GetEval().SetResult();
      });
    });
    try {
      writeResult.Get();
      m_socket->EndWriteOperation();
    } catch(const std::exception&) {
      m_socket->EndWriteOperation();
      BOOST_RETHROW;
    }
  }
}

#endif
