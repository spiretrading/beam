module;
#include "Prelude.hpp"

export module Beam:MulticastSocketWriter;

export namespace Beam {

  /** Provides the Writer interface to a MulticastSocketSender. */
  class MulticastSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class MulticastSocketChannel;
      std::shared_ptr<MulticastSocket> m_socket;
      IpAddress m_destination;

      MulticastSocketWriter(
        std::shared_ptr<MulticastSocket> socket, IpAddress destination);
      MulticastSocketWriter(const MulticastSocketWriter&) = delete;
      MulticastSocketWriter& operator =(const MulticastSocketWriter&) = delete;
  };

  template<IsConstBuffer T>
  void MulticastSocketWriter::write(const T& data) {
    m_socket->get_sender().send(DatagramPacket(data, m_destination));
  }

  inline MulticastSocketWriter::MulticastSocketWriter(
    std::shared_ptr<MulticastSocket> socket, IpAddress destination)
    : m_socket(std::move(socket)),
      m_destination(std::move(destination)) {}
}

