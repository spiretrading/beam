#ifndef BEAM_NETWORK_HPP
#define BEAM_NETWORK_HPP

namespace Beam::Network {
  template<typename B> class DatagramPacket;
  class IpAddress;
  class MulticastSocket;
  class MulticastSocketChannel;
  class MulticastSocketConnection;
  class MulticastSocketReader;
  class MulticastSocketWriter;
  class SecureSocketChannel;
  class SecureSocketConnection;
  struct SecureSocketOptions;
  class SecureSocketReader;
  class SecureSocketWriter;
  class SocketException;
  class SocketIdentifier;
  class SocketThreadPool;
  class TcpServerSocket;
  class TcpSocketChannel;
  class TcpSocketConnection;
  struct TcpSocketOptions;
  class TcpSocketReader;
  class TcpSocketWriter;
  class UdpSocket;
  class UdpSocketChannel;
  class UdpSocketConnection;
  struct UdpSocketOptions;
  class UdpSocketReader;
  class UdpSocketReceiver;
  class UdpSocketSender;
  class UdpSocketWriter;
}

#endif
