#ifndef BEAM_PYTHON_TIME_SERVICE_HPP
#define BEAM_PYTHON_TIME_SERVICE_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the tz_database class.
   * @param module The module to export to.
   */
  void ExportTzDatabase(pybind11::module& module);

  /**
   * Exports the FixedTimeClient class.
   * @param module The module to export to.
   */
  void ExportFixedTimeClient(pybind11::module& module);

  /**
   * Exports the IncrementalTimeClient class.
   * @param module The module to export to.
   */
  void ExportIncrementalTimeClient(pybind11::module& module);

  /**
   * Exports the LocalTimeClient class.
   * @param module The module to export to.
   */
  void ExportLocalTimeClient(pybind11::module& module);

  /**
   * Exports the LiveNtpTimeClient class.
   * @param module The module to export to.
   */
  void ExportNtpTimeClient(pybind11::module& module);

  /**
   * Exports the TestTimeClient class.
   * @param module The module to export to.
   */
  void ExportTestTimeClient(pybind11::module& module);

  /**
   * Exports the TestTimer class.
   * @param module The module to export to.
   */
  void ExportTestTimer(pybind11::module& module);

  /**
   * Exports the TimeClient class.
   * @param module The module to export to.
   */
  void ExportTimeClient(pybind11::module& module);

  /**
   * Exports the TimeService namespace.
   * @param module The module to export to.
   */
  void ExportTimeService(pybind11::module& module);

  /**
   * Exports the TimeServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportTimeServiceTestEnvironment(pybind11::module& module);
}

#endif
