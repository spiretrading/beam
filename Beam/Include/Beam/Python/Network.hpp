#ifndef BEAM_PYTHONNETWORK_HPP
#define BEAM_PYTHONNETWORK_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the IpAddress struct.
  void ExportIpAddress();

  //! Exports the Network namespace.
  void ExportNetwork();

  //! Exports the SocketIdentifier class.
  void ExportSocketIdentifier();

  //! Exports the TcpSocketChannel class.
  void ExportTcpSocketChannel();

  //! Exports the TcpSocketConnection class.
  void ExportTcpSocketConnection();

  //! Exports the TcpSocketReader class.
  void ExportTcpSocketReader();

  //! Exports the TcpSocketWriter class.
  void ExportTcpSocketWriter();
}
}

#endif
