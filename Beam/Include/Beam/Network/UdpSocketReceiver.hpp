#ifndef BEAM_UDPSOCKETRECEIVER_HPP
#define BEAM_UDPSOCKETRECEIVER_HPP
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/noncopyable.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/Network.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketThreadPool.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Routines/Async.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"

namespace Beam {
namespace Network {

  /*! \class UdpSocketReceiver
      \brief Receives UDP datagrams.
   */
  class UdpSocketReceiver : private boost::noncopyable {
    public:

      /*! \struct Settings
          \brief Stores the settings used by this receiver.
       */
      struct Settings {

        //! The default size of a DatagramPacket.
        static const std::size_t DEFAULT_DATAGRAM_SIZE = 2 * 1024;

        //! The default size of the receive buffer.
        static const std::size_t DEFAULT_RECEIVE_BUFFER_SIZE = 8 * 1024;

        //! The timeout on receive operations.
        boost::posix_time::time_duration m_timeout;

        //! The max datagram size.
        std::size_t m_maxDatagramSize;

        //! The default size of the receive buffer.
        std::size_t m_receiveBufferSize;

        //! Constructs default settings.
        Settings();
      };

      //! Constructs a UdpSocketReceiver.
      /*!
        \param socket The socket to send the receive operations to.
      */
      UdpSocketReceiver(const std::shared_ptr<Details::UdpSocketEntry>& socket);

      ~UdpSocketReceiver();

      //! Allows receive operations to be called, must be called prior to any
      //! receive operations.
      /*!
        \param settings The Settings to apply.
      */
      void Open(const Settings& settings);

      //! Receives a DatagramPacket.
      /*!
        \param packet The DatagramPacket that was received.
        \return The size of the received packet.
      */
      template<typename Buffer>
      std::size_t Receive(Out<DatagramPacket<Buffer>> packet);

      //! Receives a DatagramPacket.
      /*!
        \param destination Where to store the packet's data.
        \param size The maximum size of the packet to receive.
        \param address The address that sent the packet.
        \return The size of the received packet.
      */
      std::size_t Receive(char* destination, std::size_t size,
        Out<IpAddress> address);

      //! Receives a DatagramPacket.
      /*!
        \param packet The DatagramPacket that was received.
        \param size The maximum size of the packet to receive.
        \return The size of the received packet.
      */
      template<typename Buffer>
      std::size_t Receive(Out<DatagramPacket<Buffer>> packet, std::size_t size);

      //! Receives a DatagramPacket.
      /*!
        \param destination Where to store the packet's data.
        \param address The address of the packet's sender.
        \return The size of the received packet.
      */
      template<typename Buffer>
      std::size_t Receive(Out<Buffer> destination, Out<IpAddress> address);

      //! Receives a DatagramPacket.
      /*!
        \param destination Where to store the packet's data.
        \param size The maximum size of the packet.
        \param address The address of the packet's sender.
        \return The size of the received packet.
      */
      template<typename Buffer>
      std::size_t Receive(Out<Buffer> destination, std::size_t size,
        Out<IpAddress> address);

    private:
      mutable Threading::Mutex m_mutex;
      bool m_isOpen;
      bool m_isDeadlinePending;
      Threading::ConditionVariable m_isCompleteCondition;
      std::shared_ptr<Details::UdpSocketEntry> m_socket;
      boost::asio::basic_waitable_timer<boost::chrono::steady_clock> m_deadline;
      Settings m_settings;

      void CheckDeadline(const boost::system::error_code& error);
  };

  inline UdpSocketReceiver::Settings::Settings()
      : m_timeout{boost::posix_time::pos_infin},
        m_maxDatagramSize{DEFAULT_DATAGRAM_SIZE},
        m_receiveBufferSize{DEFAULT_RECEIVE_BUFFER_SIZE} {}

  inline UdpSocketReceiver::UdpSocketReceiver(
      const std::shared_ptr<Details::UdpSocketEntry>& socket)
      : m_isOpen{false},
        m_isDeadlinePending{false},
        m_socket{socket},
        m_deadline{*m_socket->m_ioService} {}

  inline UdpSocketReceiver::~UdpSocketReceiver() {
    boost::unique_lock<Threading::Mutex> lock{m_mutex};
    if(!m_isDeadlinePending) {
      return;
    }
    m_isOpen = false;
    m_deadline.cancel();
    while(m_isDeadlinePending) {
      m_isCompleteCondition.wait(lock);
    }
  }

  inline void UdpSocketReceiver::Open(const Settings& settings) {
    m_settings = settings;
    boost::system::error_code errorCode;
    boost::asio::socket_base::receive_buffer_size bufferSize{
      static_cast<int>(m_settings.m_receiveBufferSize)};
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      m_socket->m_socket.set_option(bufferSize, errorCode);
    }
    if(errorCode) {
      BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
        errorCode.message()));
    }
    m_isOpen = true;
  }

  template<typename Buffer>
  std::size_t UdpSocketReceiver::Receive(Out<DatagramPacket<Buffer>> packet) {
    return Receive(Store(packet), m_settings.m_maxDatagramSize);
  }

  inline std::size_t UdpSocketReceiver::Receive(char* destination,
      std::size_t size, Out<IpAddress> address) {
    Routines::Async<std::size_t> readResult;
    boost::asio::ip::udp::endpoint senderEndpoint;
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      if(!m_socket->m_isOpen) {
        BOOST_THROW_EXCEPTION(IO::EndOfFileException{});
      }
      m_socket->m_isReadPending = true;
      m_socket->m_socket.async_receive_from(
          boost::asio::buffer(destination, size), senderEndpoint,
        [&] (const boost::system::error_code& error, std::size_t readSize) {
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
    auto hasTimeout = m_settings.m_timeout != boost::posix_time::pos_infin;
    if(hasTimeout) {
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      m_isDeadlinePending = true;
      m_deadline.expires_from_now(boost::chrono::microseconds{
        m_settings.m_timeout.total_microseconds()});
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
    } catch(...) {
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
    return Receive(Store(destination), m_settings.m_maxDatagramSize,
      Store(address));
  }

  template<typename Buffer>
  std::size_t UdpSocketReceiver::Receive(Out<Buffer> destination,
      std::size_t size, Out<IpAddress> address) {
    auto initialSize = destination->GetSize();
    auto readSize = std::min(m_settings.m_maxDatagramSize, size);
    destination->Grow(readSize);
    auto result = Receive(destination->GetMutableData() + initialSize, readSize,
      Store(address));
    destination->Shrink(readSize - result);
    return result;
  }

  inline void UdpSocketReceiver::CheckDeadline(
      const boost::system::error_code& error) {
    {
      boost::lock_guard<Threading::Mutex> lock{m_mutex};
      m_isDeadlinePending = false;
      if(!m_isOpen) {
        m_isCompleteCondition.notify_one();
        return;
      }
    }
    if(error != boost::asio::error::operation_aborted) {
      boost::system::error_code errorCode;
      {
        boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
        m_socket->m_socket.close(errorCode);
      }
    }
  }
}
}

#endif
