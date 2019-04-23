#ifndef BEAM_TCPSOCKETCONNECTION_HPP
#define BEAM_TCPSOCKETCONNECTION_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Utilities/ToString.hpp"

namespace Beam {
namespace Network {

  /*! \class TcpSocketConnection
      \brief Implements a Connection using a TCP socket.
   */
  class TcpSocketConnection : private boost::noncopyable {
    public:
      ~TcpSocketConnection();

      //! Returns the write buffer size.
      int GetWriteBufferSize() const;

      //! Sets the write buffer size.
      /*!
        \param size The size to set the write buffer to.
      */
      void SetWriteBufferSize(int size);

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
      friend class TcpSocketChannel;
      friend class TcpServerSocket;
      static const std::size_t DEFAULT_WRITE_BUFFER_SIZE = 8 * 1024;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      std::vector<IpAddress> m_addresses;
      boost::optional<IpAddress> m_interface;
      bool m_noDelayEnabled;
      int m_writeBufferSize;
      IO::OpenState m_openState;

      TcpSocketConnection(
        const std::shared_ptr<Details::TcpSocketEntry>& socket);
      TcpSocketConnection(
        const std::shared_ptr<Details::TcpSocketEntry>& socket,
        const IpAddress& address);
      TcpSocketConnection(
        const std::shared_ptr<Details::TcpSocketEntry>& socket,
        const IpAddress& address, const IpAddress& interface);
      TcpSocketConnection(
        const std::shared_ptr<Details::TcpSocketEntry>& socket,
        const std::vector<IpAddress>& addresses);
      TcpSocketConnection(
        const std::shared_ptr<Details::TcpSocketEntry>& socket,
        const std::vector<IpAddress>& addresses, const IpAddress& interface);
      void Shutdown();
      void SetOpen();
  };

  inline TcpSocketConnection::~TcpSocketConnection() {
    Close();
  }

  inline int TcpSocketConnection::GetWriteBufferSize() const {
    return m_writeBufferSize;
  }

  inline void TcpSocketConnection::SetWriteBufferSize(int size) {
    m_writeBufferSize = size;
    if(m_openState.IsOpen()) {
      boost::system::error_code errorCode;
      boost::asio::socket_base::send_buffer_size bufferSize{m_writeBufferSize};
      {
        boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
        m_socket->m_socket.set_option(bufferSize, errorCode);
      }
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
    }
  }

  inline void TcpSocketConnection::SetNoDelay(bool noDelay) {
    m_noDelayEnabled = noDelay;
    if(m_openState.IsOpen()) {
      boost::system::error_code errorCode;
      boost::asio::ip::tcp::no_delay noDelayOption{noDelay};
      {
        boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
        m_socket->m_socket.set_option(noDelayOption, errorCode);
      }
      if(errorCode) {
        BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
          errorCode.message()));
      }
    }
  }

  inline bool TcpSocketConnection::GetNoDelay() const {
    boost::system::error_code errorCode;
    boost::asio::ip::tcp::no_delay noDelayOption;
    {
      boost::lock_guard<Threading::Mutex> lock{m_socket->m_mutex};
      m_socket->m_socket.get_option(noDelayOption);
    }
    if(errorCode) {
      BOOST_THROW_EXCEPTION(SocketException(errorCode.value(),
        errorCode.message()));
    }
    return noDelayOption.value();
  }

  inline void TcpSocketConnection::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    boost::system::error_code errorCode;
    for(auto& address : m_addresses) {
      errorCode.clear();
      boost::asio::ip::tcp::resolver resolver{*m_socket->m_ioService};
      boost::asio::ip::tcp::resolver::query query{address.GetHost(),
        ToString(address.GetPort())};
      boost::asio::ip::tcp::resolver::iterator end;
      auto endpointIterator = resolver.resolve(query, errorCode);
      if(errorCode) {
        m_openState.SetOpenFailure(IO::ConnectException{errorCode.message()});
        Shutdown();
      }
      errorCode = boost::asio::error::host_not_found;
      while(errorCode && endpointIterator != end) {
        boost::system::error_code closeError;
        m_socket->m_socket.close(closeError);
        if(m_interface.is_initialized()) {
          boost::asio::ip::tcp::endpoint localEndpoint{
            boost::asio::ip::address::from_string(m_interface->GetHost(),
            errorCode), m_interface->GetPort()};
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException{errorCode.message()});
            Shutdown();
          }
          m_socket->m_socket.open(boost::asio::ip::tcp::v4(), errorCode);
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException{errorCode.message()});
            Shutdown();
          }
          m_socket->m_socket.bind(localEndpoint, errorCode);
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException{errorCode.message()});
            Shutdown();
          }
        }
        m_socket->m_socket.connect(*endpointIterator, errorCode);
        ++endpointIterator;
      }
      if(!errorCode) {
        break;
      }
    }
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException{errorCode.message()});
      Shutdown();
    }
    boost::asio::socket_base::send_buffer_size bufferSize{m_writeBufferSize};
    m_socket->m_socket.set_option(bufferSize, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException{errorCode.message()});
      Shutdown();
    }
    boost::asio::ip::tcp::no_delay noDelay{m_noDelayEnabled};
    m_socket->m_socket.set_option(noDelay, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException{errorCode.message()});
      Shutdown();
    }
    m_socket->m_isOpen = true;
    m_openState.SetOpen();
  }

  inline void TcpSocketConnection::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline TcpSocketConnection::TcpSocketConnection(
      const std::shared_ptr<Details::TcpSocketEntry>& socket)
      : m_socket{socket},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {}

  inline TcpSocketConnection::TcpSocketConnection(
      const std::shared_ptr<Details::TcpSocketEntry>& socket,
      const IpAddress& address)
      : m_socket{socket},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {
    m_addresses.push_back(address);
  }

  inline TcpSocketConnection::TcpSocketConnection(
      const std::shared_ptr<Details::TcpSocketEntry>& socket,
      const IpAddress& address, const IpAddress& interface)
      : m_socket{socket},
        m_interface{interface},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {
    m_addresses.push_back(address);
  }

  inline TcpSocketConnection::TcpSocketConnection(
      const std::shared_ptr<Details::TcpSocketEntry>& socket,
      const std::vector<IpAddress>& addresses)
      : m_socket{socket},
        m_addresses{addresses},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {}

  inline TcpSocketConnection::TcpSocketConnection(
      const std::shared_ptr<Details::TcpSocketEntry>& socket,
      const std::vector<IpAddress>& addresses, const IpAddress& interface)
      : m_socket{socket},
        m_addresses{addresses},
        m_interface{interface},
        m_noDelayEnabled{false},
        m_writeBufferSize{DEFAULT_WRITE_BUFFER_SIZE} {}

  inline void TcpSocketConnection::Shutdown() {
    m_socket->Close();
    m_openState.SetClosed();
  }

  inline void TcpSocketConnection::SetOpen() {
    assert(m_openState.IsClosed());
    m_openState.SetOpen();
    m_socket->m_isOpen = true;
  }
}

  template<>
  struct ImplementsConcept<Network::TcpSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
