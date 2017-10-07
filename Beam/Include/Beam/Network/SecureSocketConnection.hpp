#ifndef BEAM_SECURESOCKETCONNECTION_HPP
#define BEAM_SECURESOCKETCONNECTION_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace Network {

  /*! \class SecureSocketConnection
      \brief Implements a Connection using an SSL socket over TCP.
   */
  class SecureSocketConnection : private boost::noncopyable {
    public:
      ~SecureSocketConnection();

      //! Returns the write buffer size.
      std::size_t GetWriteBufferSize() const;

      //! Sets the write buffer size.
      /*!
        \param size The size to set the write buffer to.
      */
      void SetWriteBufferSize(std::size_t size);

      //! Sets the TCP no delay option.
      /*!
        \param noDelay <code>true</code> iff the TCP no delay option should be
                       enabled.
      */
      void SetNoDelay(bool noDelay);

      //! Gets the TCP no delay option.
      bool GetNoDelay() const;

      void Open();

      void Close();

    private:
      friend class SecureSocketChannel;
      friend class SecureServerSocket;
      static const std::size_t DEFAULT_WRITE_BUFFER_SIZE = 8 * 1024;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;
      std::vector<IpAddress> m_addresses;
      boost::optional<IpAddress> m_interface;
      bool m_noDelayEnabled;
      std::size_t m_writeBufferSize;
      IO::OpenState m_openState;

      SecureSocketConnection(
        const std::shared_ptr<Details::SecureSocketEntry>& socket);
      SecureSocketConnection(
        const std::shared_ptr<Details::SecureSocketEntry>& socket,
        const IpAddress& address);
      SecureSocketConnection(
        const std::shared_ptr<Details::SecureSocketEntry>& socket,
        const IpAddress& address, const IpAddress& interface);
      SecureSocketConnection(
        const std::shared_ptr<Details::SecureSocketEntry>& socket,
        const std::vector<IpAddress>& addresses);
      SecureSocketConnection(
        const std::shared_ptr<Details::SecureSocketEntry>& socket,
        const std::vector<IpAddress>& addresses, const IpAddress& interface);
      void Shutdown();
      void SetOpen();
  };

  inline SecureSocketConnection::~SecureSocketConnection() {
    Close();
  }

  inline std::size_t SecureSocketConnection::GetWriteBufferSize() const {
    return m_writeBufferSize;
  }

  inline void SecureSocketConnection::SetWriteBufferSize(std::size_t size) {
    m_writeBufferSize = size;
    if(m_openState.IsOpen()) {
      boost::system::error_code errorCode;
      boost::asio::socket_base::send_buffer_size bufferSize(m_writeBufferSize);
      {
        boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
        m_socket->m_socket.lowest_layer().set_option(bufferSize, errorCode);
      }
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
    }
  }

  inline void SecureSocketConnection::SetNoDelay(bool noDelay) {
    m_noDelayEnabled = noDelay;
    if(m_openState.IsOpen()) {
      boost::system::error_code errorCode;
      boost::asio::ip::tcp::no_delay noDelayOption(noDelay);
      {
        boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
        m_socket->m_socket.lowest_layer().set_option(noDelayOption, errorCode);
      }
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
    }
  }

  inline bool SecureSocketConnection::GetNoDelay() const {
    boost::system::error_code errorCode;
    boost::asio::ip::tcp::no_delay noDelayOption;
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      m_socket->m_socket.lowest_layer().get_option(noDelayOption);
    }
    if(errorCode) {
      BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
        errorCode.message()));
    }
    return noDelayOption.value();
  }

  inline void SecureSocketConnection::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    boost::system::error_code errorCode;
    for(auto& address : m_addresses) {
      errorCode.clear();
      boost::asio::ip::tcp::resolver resolver{
        m_socket->m_socket.get_io_service()};
      boost::asio::ip::tcp::resolver::query query{address.GetHost(),
        ToString(address.GetPort())};
      boost::asio::ip::tcp::resolver::iterator end;
      auto endpointIterator = resolver.resolve(query, errorCode);
      if(errorCode) {
        m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
        Shutdown();
      }
      errorCode = boost::asio::error::host_not_found;
      while(errorCode != 0 && endpointIterator != end) {
        boost::system::error_code closeError;
        m_socket->m_socket.lowest_layer().close(closeError);
        if(m_interface.is_initialized()) {
          boost::asio::ip::tcp::endpoint localEndpoint{
            boost::asio::ip::address::from_string(m_interface->GetHost(),
            errorCode), m_interface->GetPort()};
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException{errorCode.message()});
            Shutdown();
          }
          m_socket->m_socket.lowest_layer().open(boost::asio::ip::tcp::v4(),
            errorCode);
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException{errorCode.message()});
            Shutdown();
          }
          m_socket->m_socket.lowest_layer().bind(localEndpoint, errorCode);
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException{errorCode.message()});
            Shutdown();
          }
        }
        m_socket->m_socket.lowest_layer().connect(*endpointIterator, errorCode);
        ++endpointIterator;
      }
      if(!errorCode) {
        break;
      }
    }
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    boost::asio::socket_base::send_buffer_size bufferSize(m_writeBufferSize);
    m_socket->m_socket.lowest_layer().set_option(bufferSize, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    boost::asio::ip::tcp::no_delay noDelay(m_noDelayEnabled);
    m_socket->m_socket.lowest_layer().set_option(noDelay, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    m_socket->m_socket.handshake(boost::asio::ssl::stream_base::client,
      errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    m_socket->m_isOpen = true;
    m_openState.SetOpen();
  }

  inline void SecureSocketConnection::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline SecureSocketConnection::SecureSocketConnection(
      const std::shared_ptr<Details::SecureSocketEntry>& socket)
      : m_socket(socket),
        m_noDelayEnabled(false),
        m_writeBufferSize(DEFAULT_WRITE_BUFFER_SIZE) {}

  inline SecureSocketConnection::SecureSocketConnection(
      const std::shared_ptr<Details::SecureSocketEntry>& socket,
      const IpAddress& address)
      : SecureSocketConnection{socket, std::vector<IpAddress>{address}} {}

  inline SecureSocketConnection::SecureSocketConnection(
      const std::shared_ptr<Details::SecureSocketEntry>& socket,
      const IpAddress& address, const IpAddress& interface)
      : SecureSocketConnection{socket, std::vector<IpAddress>{address},
          interface} {}

  inline SecureSocketConnection::SecureSocketConnection(
      const std::shared_ptr<Details::SecureSocketEntry>& socket,
      const std::vector<IpAddress>& addresses)
      : m_socket{socket},
        m_addresses{addresses},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {
    m_socket->m_socket.set_verify_mode(boost::asio::ssl::verify_none);
  }

  inline SecureSocketConnection::SecureSocketConnection(
      const std::shared_ptr<Details::SecureSocketEntry>& socket,
      const std::vector<IpAddress>& addresses, const IpAddress& interface)
      : m_socket{socket},
        m_addresses{addresses},
        m_interface{interface},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {
    m_socket->m_socket.set_verify_mode(boost::asio::ssl::verify_none);
  }

  inline void SecureSocketConnection::Shutdown() {
    m_socket->Close();
    m_openState.SetClosed();
  }

  inline void SecureSocketConnection::SetOpen() {
    assert(m_openState.IsClosed());
    m_openState.SetOpen();
    m_socket->m_isOpen = true;
  }
}

  template<>
  struct ImplementsConcept<Network::SecureSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
