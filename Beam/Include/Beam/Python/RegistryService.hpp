#ifndef BEAM_PYTHON_REGISTRY_SERVICE_HPP
#define BEAM_PYTHON_REGISTRY_SERVICE_HPP
#include <memory>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/RegistryService/RegistryClientBox.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported RegistryClientBox. */
  BEAM_EXPORT_DLL pybind11::class_<RegistryService::RegistryClientBox>&
    GetExportedRegistryClientBox();

  /**
   * Exports the ApplicationRegistryClient.
   * @param module The module to export to.
   */
  void ExportApplicationRegistryClient(pybind11::module& module);

  /**
   * Exports the RegistryEntry struct.
   * @param module The module to export to.
   */
  void ExportRegistryEntry(pybind11::module& module);

  /**
   * Exports the RegistryService namespace.
   * @param module The module to export to.
   */
  void ExportRegistryService(pybind11::module& module);

  /**
   * Exports the RegistryServiceTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportRegistryServiceTestEnvironment(pybind11::module& module);

  /**
   * Exports a RegistryClient class.
   * @param <Client> The type of client to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported client.
   */
  template<typename Client>
  auto ExportRegistryClient(pybind11::module& module, const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
      def("load_path", &Client::LoadPath).
      def("load_parent", &Client::LoadParent).
      def("load_children", &Client::LoadChildren).
      def("make_directory", &Client::MakeDirectory).
      def("copy", &Client::Copy).
      def("move", &Client::Move).
      def("load", static_cast<IO::SharedBuffer (Client::*)(
        const RegistryService::RegistryEntry&)>(&Client::Load)).
      def("make_value", static_cast<RegistryService::RegistryEntry (Client::*)(
        const std::string&, const IO::SharedBuffer&,
        const RegistryService::RegistryEntry&)>(&Client::MakeValue)).
      def("store", static_cast<RegistryService::RegistryEntry (Client::*)(
        const std::string&, const IO::SharedBuffer&,
        const RegistryService::RegistryEntry&)>(&Client::Store)).
      def("delete", &Client::Delete).
      def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client, RegistryService::RegistryClientBox>) {
      pybind11::implicitly_convertible<
        Client, RegistryService::RegistryClientBox>();
      GetExportedRegistryClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
