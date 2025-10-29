#include "Beam/Python/UidService.hpp"
#include <pybind11/pybind11.h>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/ToPythonServiceLocatorClient.hpp"
#include "Beam/Python/ToPythonUidClient.hpp"
#include "Beam/Python/ToPythonUidDataStore.hpp"
#include "Beam/Sql/SqlConnection.hpp"
#include "Beam/UidService/ApplicationDefinitions.hpp"
#include "Beam/UidService/LocalUidDataStore.hpp"
#include "Beam/UidService/SqlUidDataStore.hpp"
#include "Beam/UidServiceTests/UidServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tests;
using namespace pybind11;

namespace {
  auto uid_client = std::unique_ptr<class_<UidClient>>();
  auto uid_data_store = std::unique_ptr<class_<UidDataStore>>();
}

class_<UidClient>& Beam::Python::get_exported_uid_client() {
  return *uid_client;
}

class_<UidDataStore>& Beam::Python::get_exported_uid_data_store() {
  return *uid_data_store;
}

void Beam::Python::export_local_uid_data_store(module& module) {
  using DataStore = ToPythonUidDataStore<LocalUidDataStore>;
  export_uid_data_store<DataStore>(module, "LocalUidDataStore").
    def(pybind11::init());
}

void Beam::Python::export_mysql_uid_data_store(module& module) {
  using DataStore = ToPythonUidDataStore<
    SqlUidDataStore<SqlConnection<Viper::MySql::Connection>>>;
  export_uid_data_store<DataStore>(module, "MySqlUidDataStore").
    def(pybind11::init([] (std::string host, unsigned int port,
        std::string username, std::string password, std::string database) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::MySql::Connection>>(
          Viper::MySql::Connection(host, port, username, password, database)));
    }), call_guard<GilRelease>());
}

void Beam::Python::export_sqlite_uid_data_store(module& module) {
  using DataStore = ToPythonUidDataStore<
    SqlUidDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  export_uid_data_store<DataStore>(module, "SqliteUidDataStore").
    def(pybind11::init([] (std::string path) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(
          Viper::Sqlite3::Connection(path)));
    }), call_guard<GilRelease>());
}

void Beam::Python::export_uid_service(module& module) {
  uid_client = std::make_unique<class_<UidClient>>(
    export_uid_client<UidClient>(module, "UidClient"));
  uid_data_store = std::make_unique<class_<UidDataStore>>(
    export_uid_data_store<UidDataStore>(module, "UidDataStore"));
  export_local_uid_data_store(module);
  export_mysql_uid_data_store(module);
  export_sqlite_uid_data_store(module);
  export_uid_client<ToPythonUidClient<UidClient>>(module, "UidClientBox");
  export_uid_service_application_definitions(module);
  auto test_module = module.def_submodule("tests");
  export_uid_service_test_environment(test_module);
}

void Beam::Python::export_uid_service_application_definitions(module& module) {
  export_uid_client<ToPythonUidClient<ApplicationUidClient>>(
    module, "ApplicationUidClient").
    def(pybind11::init([] (
        ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>& client) {
      return std::make_unique<ToPythonUidClient<ApplicationUidClient>>(
        Ref(*client));
    }), keep_alive<1, 2>());
}

void Beam::Python::export_uid_service_test_environment(module& module) {
  class_<UidServiceTestEnvironment, std::shared_ptr<UidServiceTestEnvironment>>(
      module, "UidServiceTestEnvironment").
    def(pybind11::init(&make_python_shared<UidServiceTestEnvironment>),
      call_guard<GilRelease>()).
    def("close", &UidServiceTestEnvironment::close, call_guard<GilRelease>()).
    def("make_client", [] (UidServiceTestEnvironment& self) {
      return ToPythonUidClient(self.make_client());
    }, call_guard<GilRelease>());
}
