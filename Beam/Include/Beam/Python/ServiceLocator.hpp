#ifndef BEAM_PYTHON_SERVICE_LOCATOR_HPP
#define BEAM_PYTHON_SERVICE_LOCATOR_HPP
#include <memory>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/ServiceLocator/ServiceLocatorClientBox.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported ServiceLocatorClientBox. */
  BEAM_EXPORT_DLL pybind11::class_<ServiceLocator::ServiceLocatorClientBox>&
    GetExportedServiceLocatorClientBox();

  /**
   * Exports the AccountUpdate struct.
   * @param module The module to export to.
   */
  void ExportAccountUpdate(pybind11::module& module);

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
   * Exports the ServiceLocatorTestEnvironment class.
   * @param module The module to export to.
   */
  void ExportServiceLocatorTestEnvironment(pybind11::module& module);

  /**
   * Exports a ServiceLocatorClient class.
   * @param <Client> The type of client to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported client.
   */
  template<typename Client>
  auto ExportServiceLocatorClient(pybind11::module& module,
      const std::string& name) {
    auto client = pybind11::class_<Client, std::shared_ptr<Client>>(module,
      name.c_str()).
    def("get_account", &Client::GetAccount).
    def("get_session_id", &Client::GetSessionId).
    def("get_encrypted_session_id", &Client::GetEncryptedSessionId).
    def("authenticate_account", &Client::AuthenticateAccount).
    def("authenticate_session", &Client::AuthenticateSession).
    def("locate", &Client::Locate).
    def("register", &Client::Register).
    def("unregister", &Client::Unregister).
    def("load_all_accounts", &Client::LoadAllAccounts).
    def("find_account", &Client::FindAccount).
    def("make_account", &Client::MakeAccount).
    def("make_directory", &Client::MakeDirectory).
    def("store_password", &Client::StorePassword).
    def("monitor_accounts", &Client::MonitorAccounts).
    def("load_directory_entry", static_cast<ServiceLocator::DirectoryEntry (
      Client::*)(const ServiceLocator::DirectoryEntry&, const std::string&)>(
      &Client::LoadDirectoryEntry)).
    def("load_directory_entry", static_cast<ServiceLocator::DirectoryEntry (
      Client::*)(unsigned int)>(&Client::LoadDirectoryEntry)).
    def("load_parents", &Client::LoadParents).
    def("load_children", &Client::LoadChildren).
    def("delete", &Client::Delete).
    def("associate", &Client::Associate).
    def("detach", &Client::Detach).
    def("has_permissions", &Client::HasPermissions).
    def("store_permissions", &Client::StorePermissions).
    def("load_registration_time", &Client::LoadRegistrationTime).
    def("load_last_login_time", &Client::LoadLastLoginTime).
    def("rename", &Client::Rename).
    def("close", &Client::Close);
    if constexpr(!std::is_same_v<Client,
        ServiceLocator::ServiceLocatorClientBox>) {
      pybind11::implicitly_convertible<Client,
        ServiceLocator::ServiceLocatorClientBox>();
      GetExportedServiceLocatorClientBox().def(
        pybind11::init<std::shared_ptr<Client>>());
    }
    return client;
  }
}

#endif
