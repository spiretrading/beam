#ifndef BEAM_PYTHON_TIME_SERVICE_HPP
#define BEAM_PYTHON_TIME_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/TimeService/TimeClientBox.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported TimeClientBox. */
  BEAM_EXPORT_DLL pybind11::class_<TimeService::TimeClientBox>&
    GetExportedTimeClientBox();

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
   * Exports the TimeService namespace.
   * @param module The module to export to.
   */
  void ExportTimeService(pybind11::module& module);

  /**
   * Exports the TimeServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportTimeServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports a TimeClient class.
   * @param <Client> The type of TimeClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported TimeClient.
   */
  template<typename Client>
  auto ExportTimeClient(pybind11::module& module, const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("get_time", &Client::GetTime).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client, TimeService::TimeClientBox>) {
      pybind11::implicitly_convertible<Client, TimeService::TimeClientBox>();
      GetExportedTimeClientBox().def(pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
