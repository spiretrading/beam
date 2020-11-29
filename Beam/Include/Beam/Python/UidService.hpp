#ifndef BEAM_PYTHON_UID_SERVICE_HPP
#define BEAM_PYTHON_UID_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/UidService/UidClientBox.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported UidClientBox. */
  BEAM_EXPORT_DLL pybind11::class_<UidService::UidClientBox>&
    GetExportedUidClientBox();

  /**
   * Exports the ApplicationUidClient class.
   * @param module The module to export to.
   */
  void ExportApplicationUidClient(pybind11::module& module);

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

  /**
   * Exports a UidClient class.
   * @param <Client> The type of UidClient to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported UidClient.
   */
  template<typename Client>
  auto ExportUidClient(pybind11::module& module, const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load_next_uid", &Client::LoadNextUid).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client, UidService::UidClientBox>) {
      pybind11::implicitly_convertible<Client, UidService::UidClientBox>();
      GetExportedUidClientBox().def(pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
