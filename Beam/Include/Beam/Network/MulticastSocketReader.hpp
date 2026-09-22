#ifndef BEAM_MULTICAST_SOCKET_READER_HPP
#define BEAM_MULTICAST_SOCKET_READER_HPP
#include "Beam/IO/Reader.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"

namespace Beam {

  /** Implements the Reader interface for a MulticastSocketReceiver. */
  class MulticastSocketReader {
    public:
      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;
      IpAddress m_destination;

      MulticastSocketReader(
        std::shared_ptr<MulticastSocket> socket, IpAddress destination);
      MulticastSocketReader(const MulticastSocketReader&) = delete;
      MulticastSocketReader& operator =(const MulticastSocketReader&) = delete;
  };

  inline bool MulticastSocketReader::poll() const {
    return m_socket->get_receiver().poll();
  }

  template<IsBuffer R>
  std::size_t MulticastSocketReader::read(
      Out<R> destination, std::size_t size) {
    return m_socket->get_receiver().receive(
      out(destination), size, out(m_destination));
  }

  inline MulticastSocketReader::MulticastSocketReader(
    std::shared_ptr<MulticastSocket> socket, IpAddress destination)
    : m_socket(std::move(socket)),
      m_destination(std::move(destination)) {}
}

#endif
