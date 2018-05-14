#ifndef BEAM_UDPSOCKETSENDER_HPP
#define BEAM_UDPSOCKETSENDER_HPP
#include <boost/asio/ip/udp.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/TaskRunner.hpp"

namespace Beam {
namespace Network {

  /*! \class UdpSocketSender
      \brief Sends datagrams via a UDP socket.
   */
  class UdpSocketSender : private boost::noncopyable {
    public:

      //! Constructs a UdpSocketSender.
      /*!
        \param socket The socket to send the data to.
      */
      UdpSocketSender(const std::shared_ptr<Details::UdpSocketEntry>& socket);

      //! Sends a DatagramPacket.
      /*!
        \param packet The DatagramPacket to send.
      */
      template<typename Buffer>
      void Send(const DatagramPacket<Buffer>& packet);

      //! Sends a DatagramPacket.
      /*!
        \param data A pointer to the packet's data.
        \param size The size of the packet.
        \param destination The packet's destination.
      */
      void Send(const void* data, std::size_t size,
        const IpAddress& destination);

    private:
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      Threading::TaskRunner m_tasks;
  };

  inline UdpSocketSender::UdpSocketSender(
      const std::shared_ptr<Details::UdpSocketEntry>& socket)
      : m_socket(socket) {}

  template<typename Buffer>
  void UdpSocketSender::Send(const DatagramPacket<Buffer>& packet) {
    Send(packet.GetData().GetData(), packet.GetData().GetSize(),
      packet.GetAddress());
  }

  inline void UdpSocketSender::Send(const void* data, std::size_t size,
      const IpAddress& destination) {
    Routines::Async<void> writeResult;
    m_socket->BeginWriteOperation();
    m_tasks.Add(
      [&] {
        boost::asio::ip::udp::endpoint destinationEndpoint(
          boost::asio::ip::address::from_string(destination.GetHost()),
          destination.GetPort());
        m_socket->m_socket.async_send_to(boost::asio::buffer(data, size),
            destinationEndpoint,
          [&] (const boost::system::error_code& error, std::size_t writeSize) {
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
    } catch(...) {
      m_socket->EndWriteOperation();
      BOOST_RETHROW;
    }
  }
}
}

#endif
