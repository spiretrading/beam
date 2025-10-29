#ifndef BEAM_PYTHON_UID_SERVICE_HPP
#define BEAM_PYTHON_UID_SERVICE_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidDataStore.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported UidClient. */
  BEAM_EXPORT_DLL pybind11::class_<UidClient>& get_exported_uid_client();

  /** Returns the exported UidDataStore. */
  BEAM_EXPORT_DLL pybind11::class_<UidDataStore>& get_exported_uid_data_store();

  /** Exports the LocalUidDataStore class. */
  void export_local_uid_data_store(pybind11::module& module);

  /** Exports the MySqlUidDataStore class. */
  void export_mysql_uid_data_store(pybind11::module& module);

  /** Exports the SqliteUidDataStore class. */
  void export_sqlite_uid_data_store(pybind11::module& module);

  /** Exports a UidClient class. */
  template<IsUidClient T>
  auto export_uid_client(pybind11::module& module, std::string_view name) {
    auto client = pybind11::class_<T>(module, name.data()).
      def("load_next_uid", &T::load_next_uid).
      def("close", &T::close);
    if constexpr(!std::is_same_v<T, UidClient>) {
      pybind11::implicitly_convertible<T, UidClient>();
      get_exported_uid_client().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
    }
    return client;
  }

  /** Exports a UidDataStore class. */
  template<IsUidDataStore T>
  auto export_uid_data_store(pybind11::module& module, std::string_view name) {
    auto data_store = pybind11::class_<T>(module, name.data()).
      def("get_next_uid", &T::get_next_uid).
      def("reserve", &T::reserve).
      def("close", &T::close);
    if constexpr(!std::is_same_v<T, UidDataStore>) {
      pybind11::implicitly_convertible<T, UidDataStore>();
      get_exported_uid_data_store().
        def(pybind11::init<T*>(), pybind11::keep_alive<1, 2>());
    }
    return data_store;
  }

  /** Exports the UidService namespace. */
  void export_uid_service(pybind11::module& module);

  /** Exports the application definitions. */
  void export_uid_service_application_definitions(pybind11::module& module);

  /** Exports the UidServiceTestEnvironment class. */
  void export_uid_service_test_environment(pybind11::module& module);
}

#endif
