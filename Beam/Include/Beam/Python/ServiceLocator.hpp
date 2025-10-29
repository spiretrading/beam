#ifndef BEAM_PYTHON_SERVICE_LOCATOR_HPP
#define BEAM_PYTHON_SERVICE_LOCATOR_HPP
#include <memory>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/Python/DateTime.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStore.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported ServiceLocatorClient. */
  BEAM_EXPORT_DLL pybind11::class_<ServiceLocatorClient>&
    get_exported_service_locator_client();

  /** Returns the exported ServiceLocatorDataStore. */
  BEAM_EXPORT_DLL pybind11::class_<ServiceLocatorDataStore>&
    get_exported_service_locator_data_store();

  /** Exports the AccountUpdate struct. */
  void export_account_update(pybind11::module& module);

  /** Exports the CachedServiceLocatorDataStore class. */
  void export_cached_service_locator_data_store(pybind11::module& module);

  /** Exports the DirectoryEntry struct. */
  void export_directory_entry(pybind11::module& module);

  /** Exports the LocalServiceLocatorDataStore class. */
  void export_local_service_locator_data_store(pybind11::module& module);

  /** Exports the MySqlServiceLocatorDataStore class. */
  void export_mysql_service_locator_data_store(pybind11::module& module);

  /** Exports the Permissions enum. */
  void export_permissions(pybind11::module& module);

  /** Exports the ServiceEntry struct. */
  void export_service_entry(pybind11::module& module);

  /** Exports the ServiceLocator namespace. */
  void export_service_locator(pybind11::module& module);

  /** Exports the application definitions. */
  void export_service_locator_application_definitions(pybind11::module& module);

  /** Exports a ServiceLocatorClient class. */
  template<IsServiceLocatorClient T>
  auto export_service_locator_client(
      pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<T>(module, name.data()).
      def_property_readonly("account", &T::get_account).
      def_property_readonly("session_id", &T::get_session_id).
      def("get_encrypted_session_id", &T::get_encrypted_session_id).
      def("authenticate_account", &T::authenticate_account).
      def("authenticate_session", &T::authenticate_session).
      def("locate", &T::locate).
      def("add", &T::add).
      def("remove",
        pybind11::overload_cast<const ServiceEntry&>(&T::remove)).
      def("load_all_accounts", &T::load_all_accounts).
      def("find_account", &T::find_account).
      def("make_account", &T::make_account).
      def("make_directory", &T::make_directory).
      def("store_password", &T::store_password).
      def("monitor", &T::monitor).
      def("load_directory_entry",
        pybind11::overload_cast<const DirectoryEntry&, const std::string&>(
          &T::load_directory_entry)).
      def("load_directory_entry",
        pybind11::overload_cast<unsigned int>(&T::load_directory_entry)).
      def("load_parents", &T::load_parents).
      def("load_children", &T::load_children).
      def("remove",
        pybind11::overload_cast<const DirectoryEntry&>(&T::remove)).
      def("associate", &T::associate).
      def("detach", &T::detach).
      def("has_permissions", &T::has_permissions).
      def("store", &T::store).
      def("load_registration_time", &T::load_registration_time).
      def("load_last_login_time", &T::load_last_login_time).
      def("rename", &T::rename).
      def("close", &T::close);
    if constexpr(!std::is_same_v<T, ServiceLocatorClient>) {
      pybind11::implicitly_convertible<T, ServiceLocatorClient>();
      get_exported_service_locator_client().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /** Exports a ServiceLocatorDataStore class. */
  template<IsServiceLocatorDataStore T>
  auto export_service_locator_data_store(
      pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<T>(module, name.data()).
      def("load_parents", &T::load_parents).
      def("load_children", &T::load_children).
      def("load_directory_entry", &T::load_directory_entry).
      def("load_all_accounts", &T::load_all_accounts).
      def("load_all_directories", &T::load_all_directories).
      def("load_account", &T::load_account).
      def("make_account", &T::make_account).
      def("make_directory", &T::make_directory).
      def("remove", &T::remove).
      def("associate", &T::associate).
      def("detach", &T::detach).
      def("load_password", &T::load_password).
      def("set_password", &T::set_password).
      def("load_permissions", &T::load_permissions).
      def("load_all_permissions", &T::load_all_permissions).
      def("set_permissions", &T::set_permissions).
      def("load_registration_time", &T::load_registration_time).
      def("load_last_login_time", &T::load_last_login_time).
      def("store_last_login_time", &T::store_last_login_time).
      def("rename", &T::rename).
      def("close", &T::close);
    if constexpr(!std::is_same_v<T, ServiceLocatorDataStore>) {
      pybind11::implicitly_convertible<T, ServiceLocatorDataStore>();
      get_exported_service_locator_data_store().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
    }
    return data_store;
  }

  /** Exports the ServiceLocatorTestEnvironment class. */
  void export_service_locator_test_environment(pybind11::module& module);

  /** Exports the SqliteServiceLocatorDataStore class. */
  void export_sqlite_service_locator_data_store(pybind11::module& module);
}

#endif
