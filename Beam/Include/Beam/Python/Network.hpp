#ifndef BEAM_PYTHON_NETWORK_HPP
#define BEAM_PYTHON_NETWORK_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the DatagramPacket class.
   * @param module The module to export to.
   */
  void export_datagram_packet(pybind11::module& module);

  /**
   * Exports the IpAddress struct.
   * @param module The module to export to.
   */
  void export_ip_address(pybind11::module& module);

  /**
   * Exports the MulticastSocket class.
   * @param module The module to export to.
   */
  void export_multicast_socket(pybind11::module& module);

  /**
   * Exports the MulticastSocketChannel class.
   * @param module The module to export to.
   */
  void export_multicast_socket_channel(pybind11::module& module);

  /**
   * Exports the MulticastSocketConnection class.
   * @param module The module to export to.
   */
  void export_multicast_socket_connection(pybind11::module& module);

  /**
   * Exports the MulticastSocketOptions class.
   * @param module The module to export to.
   */
  void export_multicast_socket_options(pybind11::module& module);

  /**
   * Exports the MulticastSocketReader class.
   * @param module The module to export to.
   */
  void export_multicast_socket_reader(pybind11::module& module);

  /**
   * Exports the MulticastSocketWriter class.
   * @param module The module to export to.
   */
  void export_multicast_socket_writer(pybind11::module& module);

  /**
   * Exports the Network namespace.
   * @param module The module to export to.
   */
  void export_network(pybind11::module& module);

  /**
   * Exports the SecureSocketChannel class.
   * @param module The module to export to.
   */
  void export_secure_socket_channel(pybind11::module& module);

  /**
   * Exports the SecureSocketConnection class.
   * @param module The module to export to.
   */
  void export_secure_socket_connection(pybind11::module& module);

  /**
   * Exports the SecureSocketOptions class.
   * @param module The module to export to.
   */
  void export_secure_socket_options(pybind11::module& module);

  /**
   * Exports the SecureSocketReader class.
   * @param module The module to export to.
   */
  void export_secure_socket_reader(pybind11::module& module);

  /**
   * Exports the SecureSocketWriter class.
   * @param module The module to export to.
   */
  void export_secure_socket_writer(pybind11::module& module);

  /**
   * Exports the SocketIdentifier class.
   * @param module The module to export to.
   */
  void export_socket_identifier(pybind11::module& module);

  /**
   * Exports the TcpServerSocket class.
   * @param module The module to export to.
   */
  void export_tcp_server_socket(pybind11::module& module);

  /**
   * Exports the TcpSocketChannel class.
   * @param module The module to export to.
   */
  void export_tcp_socket_channel(pybind11::module& module);

  /**
   * Exports the TcpSocketConnection class.
   * @param module The module to export to.
   */
  void export_tcp_socket_connection(pybind11::module& module);

  /**
   * Exports the TcpSocketOptions class.
   * @param module The module to export to.
   */
  void export_tcp_socket_options(pybind11::module& module);

  /**
   * Exports the TcpSocketReader class.
   * @param module The module to export to.
   */
  void export_tcp_socket_reader(pybind11::module& module);

  /**
   * Exports the TcpSocketWriter class.
   * @param module The module to export to.
   */
  void export_tcp_socket_writer(pybind11::module& module);

  /**
   * Exports the UdpSocket class.
   * @param module The module to export to.
   */
  void export_udp_socket(pybind11::module& module);

  /**
   * Exports the UdpSocketChannel class.
   * @param module The module to export to.
   */
  void export_udp_socket_channel(pybind11::module& module);

  /**
   * Exports the UdpSocketConnection class.
   * @param module The module to export to.
   */
  void export_udp_socket_connection(pybind11::module& module);

  /**
   * Exports the UdpSocketOptions class.
   * @param module The module to export to.
   */
  void export_udp_socket_options(pybind11::module& module);

  /**
   * Exports the UdpSocketReader class.
   * @param module The module to export to.
   */
  void export_udp_socket_reader(pybind11::module& module);

  /**
   * Exports the UdpSocketReceiver class.
   * @param module The module to export to.
   */
  void export_udp_socket_receiver(pybind11::module& module);

  /**
   * Exports the UdpSocketSender class.
   * @param module The module to export to.
   */
  void export_udp_socket_sender(pybind11::module& module);

  /**
   * Exports the UdpSocketWriter class.
   * @param module The module to export to.
   */
  void export_udp_socket_writer(pybind11::module& module);
}

#endif
