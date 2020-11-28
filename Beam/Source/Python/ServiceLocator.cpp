#include "Beam/Python/ServiceLocator.hpp"
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/ToPythonServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  auto serviceLocatorClientBox =
    std::unique_ptr<class_<ServiceLocatorClientBox>>();
}

class_<ServiceLocatorClientBox>&
    Beam::Python::GetExportedServiceLocatorClientBox() {
  return *serviceLocatorClientBox;
}

void Beam::Python::ExportAccountUpdate(module& module) {
  auto outer = class_<AccountUpdate>(module, "AccountUpdate").
    def(init()).
    def(init<DirectoryEntry, AccountUpdate::Type>()).
    def(init<const AccountUpdate&>()).
    def_readwrite("account", &AccountUpdate::m_account).
    def_readwrite("type", &AccountUpdate::m_type).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, AccountUpdate>);
  enum_<AccountUpdate::Type>(outer, "Type").
    value("ADDED", AccountUpdate::Type::ADDED).
    value("DELETED", AccountUpdate::Type::DELETED);
}

void Beam::Python::ExportApplicationServiceLocatorClient(module& module) {
  using PythonApplicationServiceLocatorClient = ToPythonServiceLocatorClient<
    ServiceLocatorClient<ApplicationServiceLocatorClient::SessionBuilder>>;
  ExportServiceLocatorClient<PythonApplicationServiceLocatorClient>(module,
    "ApplicationServiceLocatorClient").
    def(init([] (std::string username, std::string password,
        IpAddress address) {
      return std::make_shared<PythonApplicationServiceLocatorClient>(
        std::move(username), std::move(password),
        ApplicationServiceLocatorClient::SessionBuilder(
          std::bind(factory<std::unique_ptr<TcpSocketChannel>>(), address),
          std::bind(factory<std::unique_ptr<LiveTimer>>(), seconds(10))));
    }), call_guard<GilRelease>());
}

void Beam::Python::ExportDirectoryEntry(module& module) {
  auto outer = class_<DirectoryEntry>(module, "DirectoryEntry").
    def(init()).
    def(init<DirectoryEntry::Type, unsigned int, std::string>()).
    def(init<const DirectoryEntry&>()).
    def_readwrite("type", &DirectoryEntry::m_type).
    def_readwrite("id", &DirectoryEntry::m_id).
    def_readwrite("name", &DirectoryEntry::m_name).
    def_property_readonly_static("root_account",
      [] (const object&) { return DirectoryEntry::GetRootAccount(); }).
    def_property_readonly_static("star_directory",
      [] (const object&) { return DirectoryEntry::GetStarDirectory(); }).
    def_static("make_account", static_cast<DirectoryEntry (*)(
      unsigned int, std::string)>(&DirectoryEntry::MakeAccount)).
    def_static("make_account", static_cast<DirectoryEntry (*)(unsigned int)>(
      &DirectoryEntry::MakeAccount)).
    def_static("make_directory", static_cast<DirectoryEntry (*)(
      unsigned int, std::string)>(&DirectoryEntry::MakeDirectory)).
    def_static("make_directory", static_cast<DirectoryEntry (*)(unsigned int)>(
      &DirectoryEntry::MakeDirectory)).
    def("__hash__", static_cast<std::size_t (*)(const DirectoryEntry&)>(
      &Beam::ServiceLocator::hash_value)).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, DirectoryEntry>);
  enum_<DirectoryEntry::Type::Type>(outer, "Type").
    value("NONE", DirectoryEntry::Type::NONE).
    value("ACCOUNT", DirectoryEntry::Type::ACCOUNT).
    value("DIRECTORY", DirectoryEntry::Type::DIRECTORY);
}

void Beam::Python::ExportPermissions(module& module) {
  enum_<Permission::Type>(module, "Permission").
    value("NONE", Permission::NONE).
    value("READ", Permission::READ).
    value("MOVE", Permission::MOVE).
    value("ADMINISTRATE", Permission::ADMINISTRATE);
  ExportEnumSet<Permissions>(module, "PermissionSet");
}

void Beam::Python::ExportServiceEntry(module& module) {
  class_<ServiceEntry>(module, "ServiceEntry").
    def(init()).
    def(init<std::string, JsonObject, int, DirectoryEntry>()).
    def(init<const ServiceEntry&>()).
    def_property_readonly("name", &ServiceEntry::GetName).
    def_property_readonly("properties", &ServiceEntry::GetProperties).
    def_property_readonly("id", &ServiceEntry::GetId).
    def_property_readonly("account", &ServiceEntry::GetAccount).
    def(self == self).
    def(self != self);
}

void Beam::Python::ExportServiceLocator(module& module) {
  auto submodule = module.def_submodule("service_locator");
  ExportAccountUpdate(submodule);
  serviceLocatorClientBox = std::make_unique<class_<ServiceLocatorClientBox>>(
    ExportServiceLocatorClient<ServiceLocatorClientBox>(submodule,
    "ServiceLocatorClient"));
  ExportServiceLocatorClient<
    ToPythonServiceLocatorClient<ServiceLocatorClientBox>>(submodule,
    "ServiceLocatorClientBox");
  ExportApplicationServiceLocatorClient(submodule);
  ExportDirectoryEntry(submodule);
  ExportPermissions(submodule);
  ExportServiceEntry(submodule);
  ExportQueueSuite<AccountUpdate>(submodule, "AccountUpdate");
  auto test_module = submodule.def_submodule("tests");
  ExportServiceLocatorTestEnvironment(test_module);
}

void Beam::Python::ExportServiceLocatorTestEnvironment(module& module) {
  class_<ServiceLocatorTestEnvironment>(module,
    "ServiceLocatorTestEnvironment").
    def(init(), call_guard<GilRelease>()).
    def("__del__", [] (ServiceLocatorTestEnvironment& self) {
      self.Close();
    }, call_guard<GilRelease>()).
    def("close", &ServiceLocatorTestEnvironment::Close,
      call_guard<GilRelease>()).
    def("get_root", &ServiceLocatorTestEnvironment::GetRoot,
      return_value_policy::reference_internal).
    def("make_client",
      [] (ServiceLocatorTestEnvironment& self, std::string username,
          std::string password) {
        return ToPythonServiceLocatorClient(
          self.MakeClient(std::move(username), std::move(password)));
      }, call_guard<GilRelease>()).
    def("make_client",
      [] (ServiceLocatorTestEnvironment& self) {
        return ToPythonServiceLocatorClient(self.MakeClient());
      }, call_guard<GilRelease>());
}
