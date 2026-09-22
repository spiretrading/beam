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

      explicit MulticastSocketReader(std::shared_ptr<MulticastSocket> socket);
      MulticastSocketReader(const MulticastSocketReader&) = delete;
      MulticastSocketReader& operator =(const MulticastSocketReader&) = delete;
  };

  inline bool MulticastSocketReader::poll() const {
    return m_socket->get_receiver().poll();
  }

  template<IsBuffer R>
  std::size_t MulticastSocketReader::read(
      Out<R> destination, std::size_t size) {
    return m_socket->get_receiver().receive(out(destination), size, nullptr);
  }

  inline MulticastSocketReader::MulticastSocketReader(
    std::shared_ptr<MulticastSocket> socket)
    : m_socket(std::move(socket)) {}
}

#endif
