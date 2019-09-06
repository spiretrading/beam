#ifndef BEAM_PYTHON_UID_SERVICE_HPP
#define BEAM_PYTHON_UID_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the ApplicationUidClient class.
   * @param module The module to export to.
   */
  void ExportApplicationUidClient(pybind11::module& module);

  /**
   * Exports the UidClient class.
   * @param module The module to export to.
   */
  void ExportUidClient(pybind11::module& module);

  /**
   * Exports the UidService namespace.
   * @param module The module to export to.
   */
  void ExportUidService(pybind11::module& module);

  /**
   * Exports the UidServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportUidServiceTestEnvironment(pybind11::module& module);
}

#endif
