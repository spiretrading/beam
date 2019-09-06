#ifndef BEAM_PYTHON_SERVICE_LOCATOR_HPP
#define BEAM_PYTHON_SERVICE_LOCATOR_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the ApplicationServiceLocatorClient class.
   * @param module The module to export to.
   */
  void ExportApplicationServiceLocatorClient(pybind11::module& module);

  /**
   * Exports the DirectoryEntry struct.
   * @param module The module to export to.
   */
  void ExportDirectoryEntry(pybind11::module& module);

  /**
   * Exports the Permissions enum.
   * @param module The module to export to.
   */
  void ExportPermissions(pybind11::module& module);

  /**
   * Exports the ServiceEntry struct.
   * @param module The module to export to.
   */
  void ExportServiceEntry(pybind11::module& module);

  /**
   * Exports the ServiceLocator namespace.
   * @param module The module to export to.
   */
  void ExportServiceLocator(pybind11::module& module);

  /**
   * Exports the ServiceLocatorClient class.
   * @param module The module to export to.
   */
  void ExportServiceLocatorClient(pybind11::module& module);

  /**
   * Exports the ServiceLocatorTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportServiceLocatorTestEnvironment(pybind11::module& module);
}

#endif
