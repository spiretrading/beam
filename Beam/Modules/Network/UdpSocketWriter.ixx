module;
#include "Prelude.hpp"

export module Beam:UdpSocketWriter;

import :DatagramPacket;
import :UdpSocket;
import :UdpSocketSender;

export namespace Beam {

  /** Provides the Writer interface to a UdpSocketSender. */
  class UdpSocketWriter {
    public:
      template<IsConstBuffer T>
      void write(const T& data);

    private:
      friend class UdpSocketChannel;
      std::shared_ptr<UdpSocket> m_socket;

      UdpSocketWriter(std::shared_ptr<UdpSocket> socket);
      UdpSocketWriter(const UdpSocketWriter&) = delete;
      UdpSocketWriter& operator =(const UdpSocketWriter&) = delete;
  };

  template<IsConstBuffer T>
  void UdpSocketWriter::write(const T& data) {
    m_socket->get_sender().send(DatagramPacket(data, m_socket->get_address()));
  }

  inline UdpSocketWriter::UdpSocketWriter(std::shared_ptr<UdpSocket> socket)
    : m_socket(std::move(socket)) {}
}

