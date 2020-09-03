#ifndef BEAM_UDP_SOCKET_RECEIVER_HPP
#define BEAM_UDP_SOCKET_RECEIVER_HPP
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Network/UdpSocketOptions.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam::Network {

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
       * @return The size of the received packet.
       */
      template<typename Buffer>
      std::size_t Receive(Out<DatagramPacket<Buffer>> packet);

      /**
       * Receives a DatagramPacket.
       * @param destination Where to store the packet's data.
       * @param size The maximum size of the packet to receive.
       * @param address The address that sent the packet.
       * @return The size of the received packet.
       */
      std::size_t Receive(char* destination, std::size_t size,
        Out<IpAddress> address);

      /**
       * Receives a DatagramPacket.
       * @param packet The DatagramPacket that was received.
       * @param size The maximum size of the packet to receive.
       * @return The size of the received packet.
       */
      template<typename Buffer>
      std::size_t Receive(Out<DatagramPacket<Buffer>> packet, std::size_t size);

      /**
       * Receives a DatagramPacket.
       * @param destination Where to store the packet's data.
       * @param address The address of the packet's sender.
       * @return The size of the received packet.
       */
      template<typename Buffer>
      std::size_t Receive(Out<Buffer> destination, Out<IpAddress> address);

      /**
       * Receives a DatagramPacket.
       * @param destination Where to store the packet's data.
       * @param size The maximum size of the packet.
       * @param address The address of the packet's sender.
       * @return The size of the received packet.
       */
      template<typename Buffer>
      std::size_t Receive(Out<Buffer> destination, std::size_t size,
        Out<IpAddress> address);

    private:
      mutable Threading::Mutex m_mutex;
      bool m_isOpen;
      bool m_isDeadlinePending;
      Threading::ConditionVariable m_isCompleteCondition;
      UdpSocketOptions m_options;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::asio::basic_waitable_timer<boost::chrono::steady_clock> m_deadline;

      UdpSocketReceiver(const UdpSocketReceiver&) = delete;
      UdpSocketReceiver& operator =(const UdpSocketReceiver&) = delete;
      void CheckDeadline(const boost::system::error_code& error);
  };

  inline UdpSocketReceiver::UdpSocketReceiver(const UdpSocketOptions& options,
    std::shared_ptr<Details::UdpSocketEntry> socket)
    : m_isOpen(true),
      m_isDeadlinePending(false),
      m_options(options),
      m_socket(std::move(socket)),
      m_deadline(*m_socket->m_ioService) {
    auto errorCode = boost::system::error_code();
    auto bufferSize = boost::asio::socket_base::receive_buffer_size(
      static_cast<int>(m_options.m_receiveBufferSize));
    m_socket->m_socket.set_option(bufferSize, errorCode);
    if(errorCode) {
      BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
        errorCode.message()));
    }
  }

  inline UdpSocketReceiver::~UdpSocketReceiver() {
    auto lock = boost::unique_lock(m_mutex);
    if(!m_isDeadlinePending) {
      return;
    }
    m_isOpen = false;
    m_deadline.cancel();
    while(m_isDeadlinePending) {
      m_isCompleteCondition.wait(lock);
    }
  }

  template<typename Buffer>
  std::size_t UdpSocketReceiver::Receive(Out<DatagramPacket<Buffer>> packet) {
    return Receive(Store(packet), m_options.m_maxDatagramSize);
  }

  inline std::size_t UdpSocketReceiver::Receive(char* destination,
      std::size_t size, Out<IpAddress> address) {
    auto readResult = Routines::Async<std::size_t>();
    auto senderEndpoint = boost::asio::ip::udp::endpoint();
    {
      auto lock = boost::lock_guard(m_socket->m_mutex);
      if(!m_socket->m_isOpen) {
        BOOST_THROW_EXCEPTION(IO::EndOfFileException());
      }
      m_socket->m_isReadPending = true;
      m_socket->m_socket.async_receive_from(
          boost::asio::buffer(destination, size), senderEndpoint,
        [&] (const auto& error, auto readSize) {
          if(error) {
            if(Details::IsEndOfFile(error)) {
              readResult.GetEval().SetException(IO::EndOfFileException());
              return;
            }
            readResult.GetEval().SetException(SocketException(error.value(),
              error.message()));
            return;
          }
          *address = IpAddress(senderEndpoint.address().to_string(),
            senderEndpoint.port());
          readResult.GetEval().SetResult(readSize);
        });
    }
    auto hasTimeout = m_options.m_timeout != boost::posix_time::pos_infin;
    if(hasTimeout) {
      auto lock = boost::lock_guard(m_mutex);
      m_isDeadlinePending = true;
      m_deadline.expires_from_now(boost::chrono::microseconds{
        m_options.m_timeout.total_microseconds()});
      m_deadline.async_wait(std::bind(&UdpSocketReceiver::CheckDeadline, this,
        std::placeholders::_1));
    }
    try {
      auto result = readResult.Get();
      if(hasTimeout) {
        m_deadline.cancel();
      }
      m_socket->EndReadOperation();
      return result;
    } catch(const std::exception&) {
      m_socket->EndReadOperation();
      BOOST_RETHROW;
    }
  }

  template<typename Buffer>
  std::size_t UdpSocketReceiver::Receive(Out<DatagramPacket<Buffer>> packet,
      std::size_t size) {
    return Receive(Store(packet->GetData()), size, Store(packet->GetAddress()));
  }

  template<typename Buffer>
  std::size_t UdpSocketReceiver::Receive(Out<Buffer> destination,
      Out<IpAddress> address) {
    return Receive(Store(destination), m_options.m_maxDatagramSize,
      Store(address));
  }

  template<typename Buffer>
  std::size_t UdpSocketReceiver::Receive(Out<Buffer> destination,
      std::size_t size, Out<IpAddress> address) {
    auto initialSize = destination->GetSize();
    auto readSize = std::min(m_options.m_maxDatagramSize, size);
    destination->Grow(readSize);
    auto result = Receive(destination->GetMutableData() + initialSize, readSize,
      Store(address));
    destination->Shrink(readSize - result);
    return result;
  }

  inline void UdpSocketReceiver::CheckDeadline(
      const boost::system::error_code& error) {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_isDeadlinePending = false;
      if(!m_isOpen) {
        m_isCompleteCondition.notify_one();
        return;
      }
    }
    if(error != boost::asio::error::operation_aborted) {
      auto errorCode = boost::system::error_code();
      {
        auto lock = boost::lock_guard(m_socket->m_mutex);
        m_socket->m_socket.close(errorCode);
      }
    }
  }
}

#endif
