module;
#include "Prelude.hpp"

export module Beam:MulticastSocketConnection;

import :MulticastSocket;

export namespace Beam {

  /** Provides a Connection interface for a MulticastSocket. */
  class MulticastSocketConnection {
    public:
      ~MulticastSocketConnection();

      void close();

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;

      MulticastSocketConnection(std::shared_ptr<MulticastSocket> socket);
      MulticastSocketConnection(const MulticastSocketConnection&) = delete;
      MulticastSocketConnection& operator =(
        const MulticastSocketConnection&) = delete;
  };

  inline MulticastSocketConnection::~MulticastSocketConnection() {
    close();
  }

  inline void MulticastSocketConnection::close() {
    m_socket->close();
  }

  inline MulticastSocketConnection::MulticastSocketConnection(
    std::shared_ptr<MulticastSocket> socket)
    : m_socket(std::move(socket)) {}
}

