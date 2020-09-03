#ifndef BEAM_PYTHON_NETWORK_HPP
#define BEAM_PYTHON_NETWORK_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the DatagramPacket class.
   * @param module The module to export to.
   */
  void ExportDatagramPacket(pybind11::module& module);

  /**
   * Exports the IpAddress struct.
   * @param module The module to export to.
   */
  void ExportIpAddress(pybind11::module& module);

  /**
   * Exports the MulticastSocket class.
   * @param module The module to export to.
   */
  void ExportMulticastSocket(pybind11::module& module);

  /**
   * Exports the MulticastSocketChannel class.
   * @param module The module to export to.
   */
  void ExportMulticastSocketChannel(pybind11::module& module);

  /**
   * Exports the MulticastSocketConnection class.
   * @param module The module to export to.
   */
  void ExportMulticastSocketConnection(pybind11::module& module);

  /**
   * Exports the MulticastSocketOptions class.
   * @param module The module to export to.
   */
  void ExportMulticastSocketOptions(pybind11::module& module);

  /**
   * Exports the MulticastSocketReader class.
   * @param module The module to export to.
   */
  void ExportMulticastSocketReader(pybind11::module& module);

  /**
   * Exports the MulticastSocketWriter class.
   * @param module The module to export to.
   */
  void ExportMulticastSocketWriter(pybind11::module& module);

  /**
   * Exports the Network namespace.
   * @param module The module to export to.
   */
  void ExportNetwork(pybind11::module& module);

  /**
   * Exports the SecureSocketChannel class.
   * @param module The module to export to.
   */
  void ExportSecureSocketChannel(pybind11::module& module);

  /**
   * Exports the SecureSocketConnection class.
   * @param module The module to export to.
   */
  void ExportSecureSocketConnection(pybind11::module& module);

  /**
   * Exports the SecureSocketOptions class.
   * @param module The module to export to.
   */
  void ExportSecureSocketOptions(pybind11::module& module);

  /**
   * Exports the SecureSocketReader class.
   * @param module The module to export to.
   */
  void ExportSecureSocketReader(pybind11::module& module);

  /**
   * Exports the SecureSocketWriter class.
   * @param module The module to export to.
   */
  void ExportSecureSocketWriter(pybind11::module& module);

  /**
   * Exports the SocketIdentifier class.
   * @param module The module to export to.
   */
  void ExportSocketIdentifier(pybind11::module& module);

  /**
   * Exports the TcpServerSocket class.
   * @param module The module to export to.
   */
  void ExportTcpServerSocket(pybind11::module& module);

  /**
   * Exports the TcpSocketChannel class.
   * @param module The module to export to.
   */
  void ExportTcpSocketChannel(pybind11::module& module);

  /**
   * Exports the TcpSocketConnection class.
   * @param module The module to export to.
   */
  void ExportTcpSocketConnection(pybind11::module& module);

  /**
   * Exports the TcpSocketOptions class.
   * @param module The module to export to.
   */
  void ExportTcpSocketOptions(pybind11::module& module);

  /**
   * Exports the TcpSocketReader class.
   * @param module The module to export to.
   */
  void ExportTcpSocketReader(pybind11::module& module);

  /**
   * Exports the TcpSocketWriter class.
   * @param module The module to export to.
   */
  void ExportTcpSocketWriter(pybind11::module& module);

  /**
   * Exports the UdpSocket class.
   * @param module The module to export to.
   */
  void ExportUdpSocket(pybind11::module& module);

  /**
   * Exports the UdpSocketChannel class.
   * @param module The module to export to.
   */
  void ExportUdpSocketChannel(pybind11::module& module);

  /**
   * Exports the UdpSocketConnection class.
   * @param module The module to export to.
   */
  void ExportUdpSocketConnection(pybind11::module& module);

  /**
   * Exports the UdpSocketOptions class.
   * @param module The module to export to.
   */
  void ExportUdpSocketOptions(pybind11::module& module);

  /**
   * Exports the UdpSocketReader class.
   * @param module The module to export to.
   */
  void ExportUdpSocketReader(pybind11::module& module);

  /**
   * Exports the UdpSocketReceiver class.
   * @param module The module to export to.
   */
  void ExportUdpSocketReceiver(pybind11::module& module);

  /**
   * Exports the UdpSocketSender class.
   * @param module The module to export to.
   */
  void ExportUdpSocketSender(pybind11::module& module);

  /**
   * Exports the UdpSocketWriter class.
   * @param module The module to export to.
   */
  void ExportUdpSocketWriter(pybind11::module& module);
}

#endif
