#ifndef BEAM_PYTHON_NETWORK_HPP
#define BEAM_PYTHON_NETWORK_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the IpAddress struct.
   * @param module The module to export to.
   */
  void ExportIpAddress(pybind11::module& module);

  /**
   * Exports the Network namespace.
   * @param module The module to export to.
   */
  void ExportNetwork(pybind11::module& module);

  /**
   * Exports the SocketIdentifier class.
   * @param module The module to export to.
   */
  void ExportSocketIdentifier(pybind11::module& module);

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
   * Exports the TcpSocketReader class.
   * @param module The module to export to.
   */
  void ExportTcpSocketReader(pybind11::module& module);

  /**
   * Exports the TcpSocketWriter class.
   * @param module The module to export to.
   */
  void ExportTcpSocketWriter(pybind11::module& module);
}

#endif
