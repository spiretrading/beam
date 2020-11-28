#include "Beam/Python/ServiceLocator.hpp"
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <boost/lexical_cast.hpp>
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

void Beam::Python::ExportApplicationServiceLocatorClient(
    module& module) {
#if 0
  using PythonApplicationServiceLocatorClient =
    ServiceLocatorClient<ApplicationServiceLocatorClient::SessionBuilder>;
  class_<ToPythonServiceLocatorClient<PythonApplicationServiceLocatorClient>,
      VirtualServiceLocatorClient>(module, "ApplicationServiceLocatorClient")
    .def(init(
      [] (std::string username, std::string password, IpAddress address) {
        auto sessionBuilder = ApplicationServiceLocatorClient::SessionBuilder(
          [=] {
            return std::make_unique<TcpSocketChannel>(address);
          },
          [] {
            return std::make_unique<LiveTimer>(seconds(10));
          });
        return MakeToPythonServiceLocatorClient(
          std::make_unique<PythonApplicationServiceLocatorClient>(
          std::move(username), std::move(password), std::move(sessionBuilder)));
      }), call_guard<GilRelease>());
#endif
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
  ExportServiceLocatorClient(submodule);
  ExportApplicationServiceLocatorClient(submodule);
  ExportDirectoryEntry(submodule);
  ExportPermissions(submodule);
  ExportServiceEntry(submodule);
  auto test_module = submodule.def_submodule("tests");
  ExportServiceLocatorTestEnvironment(test_module);
}

void Beam::Python::ExportServiceLocatorClient(module& module) {
#if 0
  class_<VirtualServiceLocatorClient, TrampolineServiceLocatorClient>(module,
      "ServiceLocatorClient")
    .def("get_account", &VirtualServiceLocatorClient::GetAccount)
    .def("get_session_id", &VirtualServiceLocatorClient::GetSessionId)
    .def("get_encrypted_session_id",
      &VirtualServiceLocatorClient::GetEncryptedSessionId)
    .def("authenticate_account",
      &VirtualServiceLocatorClient::AuthenticateAccount)
    .def("authenticate_session",
      &VirtualServiceLocatorClient::AuthenticateSession)
    .def("locate", &VirtualServiceLocatorClient::Locate)
    .def("register", &VirtualServiceLocatorClient::Register)
    .def("unregister", &VirtualServiceLocatorClient::Unregister)
    .def("load_all_accounts", &VirtualServiceLocatorClient::LoadAllAccounts)
    .def("find_account", &VirtualServiceLocatorClient::FindAccount)
    .def("make_account", &VirtualServiceLocatorClient::MakeAccount)
    .def("make_directory", &VirtualServiceLocatorClient::MakeDirectory)
    .def("store_password", &VirtualServiceLocatorClient::StorePassword)
    .def("monitor_accounts", &VirtualServiceLocatorClient::MonitorAccounts)
    .def("load_directory_entry",
      static_cast<DirectoryEntry (VirtualServiceLocatorClient::*)(
      const DirectoryEntry&, const std::string&)>(
      &VirtualServiceLocatorClient::LoadDirectoryEntry))
    .def("load_directory_entry", static_cast<
      DirectoryEntry (VirtualServiceLocatorClient::*)(unsigned int)>(
      &VirtualServiceLocatorClient::LoadDirectoryEntry))
    .def("load_parents", &VirtualServiceLocatorClient::LoadParents)
    .def("load_children", &VirtualServiceLocatorClient::LoadChildren)
    .def("delete", &VirtualServiceLocatorClient::Delete)
    .def("associate", &VirtualServiceLocatorClient::Associate)
    .def("detach", &VirtualServiceLocatorClient::Detach)
    .def("has_permissions", &VirtualServiceLocatorClient::HasPermissions)
    .def("store_permissions", &VirtualServiceLocatorClient::StorePermissions)
    .def("load_registration_time",
      &VirtualServiceLocatorClient::LoadRegistrationTime)
    .def("load_last_login_time",
      &VirtualServiceLocatorClient::LoadLastLoginTime)
    .def("rename", &VirtualServiceLocatorClient::Rename)
    .def("close", &VirtualServiceLocatorClient::Close);
#endif
  ExportQueueSuite<AccountUpdate>(module, "AccountUpdate");
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
    def("build_client",
      [] (ServiceLocatorTestEnvironment& self, std::string username,
          std::string password) {
        return ToPythonServiceLocatorClient(self.MakeClient(std::move(username),
          std::move(password)));
      }, call_guard<GilRelease>()).
    def("build_client",
      [] (ServiceLocatorTestEnvironment& self) {
        return ToPythonServiceLocatorClient(self.MakeClient());
      }, call_guard<GilRelease>());
}
