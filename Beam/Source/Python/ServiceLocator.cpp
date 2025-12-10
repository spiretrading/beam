#include "Beam/Python/ServiceLocator.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <Viper/MySql/Connection.hpp>
#include <Viper/Sqlite3/Connection.hpp>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Collections.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/ToPythonServiceLocatorClient.hpp"
#include "Beam/Python/ToPythonServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/AuthenticationException.hpp"
#include "Beam/ServiceLocator/CachedServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/LocalServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocator/NotLoggedInException.hpp"
#include "Beam/ServiceLocator/ServiceLocatorDataStoreException.hpp"
#include "Beam/ServiceLocator/SqlServiceLocatorDataStore.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Beam/Sql/SqlConnection.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  auto service_locator_client = std::unique_ptr<class_<ServiceLocatorClient>>();
  auto service_locator_data_store =
    std::unique_ptr<class_<ServiceLocatorDataStore>>();
}

class_<ServiceLocatorClient>&
    Beam::Python::get_exported_service_locator_client() {
  return *service_locator_client;
}

class_<ServiceLocatorDataStore>&
    Beam::Python::get_exported_service_locator_data_store() {
  return *service_locator_data_store;
}

void Beam::Python::export_account_update(module& module) {
  auto outer = class_<AccountUpdate>(module, "AccountUpdate").
    def(pybind11::init<DirectoryEntry, AccountUpdate::Type>()).
    def_readwrite("account", &AccountUpdate::m_account).
    def_readwrite("type", &AccountUpdate::m_type).
    def_static("add", &AccountUpdate::add).
    def_static("remove", &AccountUpdate::remove);
  export_default_methods(outer);
  enum_<AccountUpdate::Type>(outer, "Type").
    value("ADDED", AccountUpdate::Type::ADDED).
    value("DELETED", AccountUpdate::Type::DELETED);
}

void Beam::Python::export_cached_service_locator_data_store(module& module) {
  using PythonCachedServiceLocatorDataStore = ToPythonServiceLocatorDataStore<
    CachedServiceLocatorDataStore<ServiceLocatorDataStore>>;
  export_service_locator_data_store<PythonCachedServiceLocatorDataStore>(
    module, "CachedServiceLocatorDataStore").
    def(pybind11::init<ServiceLocatorDataStore>());
}

void Beam::Python::export_directory_entry(module& module) {
  auto outer = class_<DirectoryEntry>(module, "DirectoryEntry").
    def(pybind11::init<DirectoryEntry::Type, unsigned int, std::string>()).
    def_readwrite("type", &DirectoryEntry::m_type).
    def_readwrite("id", &DirectoryEntry::m_id).
    def_readwrite("name", &DirectoryEntry::m_name).
    def_readonly_static("ROOT_ACCOUNT", &DirectoryEntry::ROOT_ACCOUNT).
    def_readonly_static("STAR_DIRECTORY", &DirectoryEntry::STAR_DIRECTORY).
    def_static("name_comparator", &DirectoryEntry::name_comparator).
    def_static("make_account",
      overload_cast<unsigned int, std::string>(&DirectoryEntry::make_account)).
    def_static("make_account",
      overload_cast<unsigned int>(&DirectoryEntry::make_account)).
    def_static("make_directory", overload_cast<unsigned int, std::string>(
      &DirectoryEntry::make_directory)).
    def_static("make_directory",
      overload_cast<unsigned int>(&DirectoryEntry::make_directory));
  export_default_methods(outer);
  enum_<DirectoryEntry::Type::Type>(outer, "Type").
    value("NONE", DirectoryEntry::Type::NONE).
    value("ACCOUNT", DirectoryEntry::Type::ACCOUNT).
    value("DIRECTORY", DirectoryEntry::Type::DIRECTORY);
}

void Beam::Python::export_local_service_locator_data_store(module& module) {
  using DataStore =
    ToPythonServiceLocatorDataStore<LocalServiceLocatorDataStore>;
  export_service_locator_data_store<DataStore>(
    module, "LocalServiceLocatorDataStore").
    def(pybind11::init()).
    def("store", [] (DataStore& self, DirectoryEntry entry, std::string name,
        ptime created, ptime last_modified) {
      self.get().store(
        std::move(entry), std::move(name), created, last_modified);
    }, call_guard<gil_scoped_release>()).
    def("store", [] (DataStore& self, DirectoryEntry entry) {
      self.get().store(std::move(entry));
    }, call_guard<gil_scoped_release>());
}

void Beam::Python::export_mysql_service_locator_data_store(module& module) {
  using DataStore = ToPythonServiceLocatorDataStore<
    SqlServiceLocatorDataStore<SqlConnection<Viper::MySql::Connection>>>;
  export_service_locator_data_store<DataStore>(
      module, "MySqlServiceLocatorDataStore").
    def(pybind11::init([] (std::string host, unsigned int port,
        std::string username, std::string password, std::string database) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::MySql::Connection>>(
          Viper::MySql::Connection(host, port, username, password, database)));
    }), call_guard<gil_scoped_release>());
}

void Beam::Python::export_permissions(module& module) {
  enum_<Permission::Type>(module, "Permission").
    value("NONE", Permission::NONE).
    value("READ", Permission::READ).
    value("MOVE", Permission::MOVE).
    value("ADMINISTRATE", Permission::ADMINISTRATE);
  export_enum_set<Permissions>(module, "PermissionSet");
}

void Beam::Python::export_service_entry(module& module) {
  auto entry = class_<ServiceEntry>(module, "ServiceEntry").
    def(pybind11::init<std::string, JsonObject, int, DirectoryEntry>()).
    def_property_readonly("name", &ServiceEntry::get_name).
    def_property_readonly("properties", &ServiceEntry::get_properties).
    def_property_readonly("id", &ServiceEntry::get_id).
    def_property_readonly("account", &ServiceEntry::get_account);
  export_default_methods(entry);
}

void Beam::Python::export_service_locator(module& module) {
  service_locator_client = std::make_unique<class_<ServiceLocatorClient>>(
    export_service_locator_client<ServiceLocatorClient>(
      module, "ServiceLocatorClient"));
  service_locator_data_store = std::make_unique<class_<ServiceLocatorDataStore>>(
    export_service_locator_data_store<ServiceLocatorDataStore>(
      module, "ServiceLocatorDataStore"));
  export_account_update(module);
  export_cached_service_locator_data_store(module);
  export_directory_entry(module);
  export_local_service_locator_data_store(module);
  export_mysql_service_locator_data_store(module);
  export_permissions(module);
  export_service_entry(module);
  export_service_locator_application_definitions(module);
  export_sqlite_service_locator_data_store(module);
  export_queue_suite<AccountUpdate>(module, "AccountUpdate");
  register_exception<AuthenticationException>(
    module, "AuthenticationException", get_connect_exception());
  register_exception<NotLoggedInException>(
    module, "NotLoggedInException", get_io_exception());
  register_exception<ServiceLocatorDataStoreException>(
    module, "ServiceLocatorDataStoreException", get_io_exception());
  auto test_module = module.def_submodule("tests");
  export_service_locator_test_environment(test_module);
}

void Beam::Python::export_service_locator_application_definitions(
    module& module) {
  auto client_config =
    class_<ServiceLocatorClientConfig>(module, "ServiceLocatorClientConfig").
      def(pybind11::init<
        const IpAddress&, const std::string&, const std::string&>()).
      def_readwrite("address", &ServiceLocatorClientConfig::m_address).
      def_readwrite("username", &ServiceLocatorClientConfig::m_username).
      def_readwrite("password", &ServiceLocatorClientConfig::m_password).
      def_static("parse", &ServiceLocatorClientConfig::parse);
  export_default_methods(client_config);
  auto service_config =
    class_<ServiceConfiguration>(module, "ServiceConfiguration").
      def(pybind11::init<
        const std::string&, const IpAddress&, const JsonObject&>()).
      def_readwrite("name", &ServiceConfiguration::m_name).
      def_readwrite("interface", &ServiceConfiguration::m_interface).
      def_readwrite("properties", &ServiceConfiguration::m_properties).
      def_static("parse", &ServiceConfiguration::parse);
  export_default_methods(service_config);
  export_service_locator_client<
    ToPythonServiceLocatorClient<ApplicationServiceLocatorClient>>(
      module, "ApplicationServiceLocatorClient").
    def(pybind11::init<std::string, std::string, IpAddress>());
  module.def(
    "add", &add<ServiceLocatorClient>, call_guard<gil_scoped_release>());
}

void Beam::Python::export_sqlite_service_locator_data_store(module& module) {
  using DataStore = ToPythonServiceLocatorDataStore<
    SqlServiceLocatorDataStore<SqlConnection<Viper::Sqlite3::Connection>>>;
  export_service_locator_data_store<DataStore>(
    module, "SqliteServiceLocatorDataStore").
    def(pybind11::init([] (std::string path) {
      return std::make_unique<DataStore>(
        std::make_unique<SqlConnection<Viper::Sqlite3::Connection>>(
          Viper::Sqlite3::Connection(path)));
      }), call_guard<gil_scoped_release>());
}

void Beam::Python::export_service_locator_test_environment(module& module) {
  class_<ServiceLocatorTestEnvironment, std::shared_ptr<ServiceLocatorTestEnvironment>>(
    module, "ServiceLocatorTestEnvironment").
    def(pybind11::init(&make_python_shared<ServiceLocatorTestEnvironment>),
      call_guard<gil_scoped_release>()).
    def("close", &ServiceLocatorTestEnvironment::close,
      call_guard<gil_scoped_release>()).
    def("get_root", &ServiceLocatorTestEnvironment::get_root,
      return_value_policy::reference_internal).
    def("make_client",
      [] (ServiceLocatorTestEnvironment& self, std::string username,
          std::string password) {
        return ToPythonServiceLocatorClient(
          self.make_client(std::move(username), std::move(password)));
      }, call_guard<gil_scoped_release>()).
    def("make_client", [] (ServiceLocatorTestEnvironment& self) {
      return ToPythonServiceLocatorClient(self.make_client());
    }, call_guard<gil_scoped_release>());
}
