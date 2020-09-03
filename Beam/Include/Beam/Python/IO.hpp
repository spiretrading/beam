#ifndef BEAM_PYTHON_IO_HPP
#define BEAM_PYTHON_IO_HPP
#include <pybind11/pybind11.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Python/Out.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the Python object representing an IOException. */
  BEAM_EXPORT_DLL const pybind11::object& GetIOException();

  /**
   * Exports the Channel class.
   * @param module The module to export to.
   */
  void ExportChannel(pybind11::module& module);

  /**
   * Exports the ChannelIdentifier class.
   * @param module The module to export to.
   */
  void ExportChannelIdentifier(pybind11::module& module);

  /**
   * Exports the Connection class.
   * @param module The module to export to.
   */
  void ExportConnection(pybind11::module& module);

  /**
   * Exports the IO namespace.
   * @param module The module to export to.
   */
  void ExportIO(pybind11::module& module);

  /**
   * Exports the OpenState class.
   * @param module The module to export to.
   */
  void ExportOpenState(pybind11::module& module);

  /**
   * Exports the Reader class.
   * @param module The module to export to.
   */
  void ExportReader(pybind11::module& module);

  /**
   * Exports the ServerConnection class.
   * @param module The module to export to.
   */
  void ExportServerConnection(pybind11::module& module);

  /**
   * Exports the SharedBuffer class.
   * @param module The module to export to.
   */
  void ExportSharedBuffer(pybind11::module& module);

  /**
   * Exports the Writer class.
   * @param module The module to export to.
   */
  void ExportWriter(pybind11::module& module);
}

#endif
