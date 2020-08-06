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
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
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
  struct TrampolineServiceLocatorClient final : VirtualServiceLocatorClient {
    DirectoryEntry GetAccount() const override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "get_account", GetAccount);
    }

    std::string GetSessionId() const override {
      PYBIND11_OVERLOAD_PURE_NAME(std::string, VirtualServiceLocatorClient,
        "get_session_id", GetSessionId);
    }

    std::string GetEncryptedSessionId(unsigned int key) const override {
      PYBIND11_OVERLOAD_PURE_NAME(std::string, VirtualServiceLocatorClient,
        "get_encrypted_session_id", GetEncryptedSessionId, key);
    }

    DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "authenticate_account", AuthenticateAccount, username, password);
    }

    DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "authenticate_session", AuthenticateSession, sessionId, key);
    }

    std::vector<ServiceEntry> Locate(const std::string& name) override{
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<ServiceEntry>,
        VirtualServiceLocatorClient, "locate", Locate, name);
    }

    ServiceEntry Register(const std::string& name,
        const JsonObject& properties) override {
      PYBIND11_OVERLOAD_PURE_NAME(ServiceEntry, VirtualServiceLocatorClient,
        "register", Register, name, properties);
    }

    std::vector<DirectoryEntry> LoadAllAccounts() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualServiceLocatorClient, "load_all_accounts", LoadAllAccounts);
    }

    optional<DirectoryEntry> FindAccount(
        const std::string& name) override {
      PYBIND11_OVERLOAD_PURE_NAME(optional<DirectoryEntry>,
        VirtualServiceLocatorClient, "find_account", FindAccount, name);
    }

    DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password,
        const DirectoryEntry& parent) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "make_account", MakeAccount, name, password, parent);
    }

    DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "make_directory", MakeDirectory, name, parent);
    }

    void StorePassword(const DirectoryEntry& account,
        const std::string& password) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient,
        "store_password", StorePassword, account, password);
    }

    void MonitorAccounts(ScopedQueueWriter<AccountUpdate> queue) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient,
        "monitor_accounts", MonitorAccounts, std::move(queue));
    }

    DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "load_directory_entry", LoadDirectoryEntry, root, path);
    }

    DirectoryEntry LoadDirectoryEntry(unsigned int id) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "load_directory_entry", LoadDirectoryEntry, id);
    }

    std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualServiceLocatorClient, "load_parents", LoadParents, entry);
    }

    std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry) override {
      PYBIND11_OVERLOAD_PURE_NAME(std::vector<DirectoryEntry>,
        VirtualServiceLocatorClient, "load_children", LoadChildren, entry);
    }

    void Delete(const DirectoryEntry& entry) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient, "delete",
        Delete, entry);
    }

    void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient,
        "associate", Associate, entry, parent);
    }

    void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient, "detach",
        Detach, entry, parent);
    }

    bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions) override {
      PYBIND11_OVERLOAD_PURE_NAME(bool, VirtualServiceLocatorClient,
        "has_permissions", HasPermissions, account, target, permissions);
    }

    void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient,
        "store_permissions", StorePermissions, source, target, permissions);
    }

    ptime LoadRegistrationTime(const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(ptime, VirtualServiceLocatorClient,
        "load_registration_time", LoadRegistrationTime, account);
    }

    ptime LoadLastLoginTime(const DirectoryEntry& account) override {
      PYBIND11_OVERLOAD_PURE_NAME(ptime, VirtualServiceLocatorClient,
        "load_last_login_time", LoadLastLoginTime, account);
    }

    DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name) override {
      PYBIND11_OVERLOAD_PURE_NAME(DirectoryEntry, VirtualServiceLocatorClient,
        "rename", Rename, entry, name);
    }

    void SetCredentials(const std::string& username,
        const std::string& password) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient,
        "set_credentials", SetCredentials, username, password);
    }

    void Open() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient, "open",
        Open);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualServiceLocatorClient, "close",
        Close);
    }
  };
}

void Beam::Python::ExportAccountUpdate(module& module) {
  auto outer = class_<AccountUpdate>(module, "AccountUpdate")
    .def(init())
    .def(init<DirectoryEntry, AccountUpdate::Type>())
    .def(init<const AccountUpdate&>())
    .def_readwrite("account", &AccountUpdate::m_account)
    .def_readwrite("type", &AccountUpdate::m_type)
    .def(self == self)
    .def(self != self)
    .def("__str__", &lexical_cast<std::string, AccountUpdate>);
  enum_<AccountUpdate::Type>(outer, "Type")
    .value("ADDED", AccountUpdate::Type::ADDED)
    .value("DELETED", AccountUpdate::Type::DELETED);
}

void Beam::Python::ExportApplicationServiceLocatorClient(
    pybind11::module& module) {
  using PythonApplicationServiceLocatorClient =
    ServiceLocatorClient<ApplicationServiceLocatorClient::SessionBuilder>;
  class_<ToPythonServiceLocatorClient<PythonApplicationServiceLocatorClient>,
      VirtualServiceLocatorClient>(module, "ApplicationServiceLocatorClient")
    .def(init(
      [] (const IpAddress& address) {
        auto isConnected = false;
        auto sessionBuilder = ApplicationServiceLocatorClient::SessionBuilder(
          [=] () mutable {
            if(isConnected) {
              throw NotConnectedException();
            }
            isConnected = true;
            return std::make_unique<TcpSocketChannel>(address,
              Ref(*GetSocketThreadPool()));
          },
          [] {
            return std::make_unique<LiveTimer>(seconds(10),
              Ref(*GetTimerThreadPool()));
          });
        return MakeToPythonServiceLocatorClient(
          std::make_unique<PythonApplicationServiceLocatorClient>(
          sessionBuilder));
      }));
}

void Beam::Python::ExportDirectoryEntry(pybind11::module& module) {
  auto outer = class_<DirectoryEntry>(module, "DirectoryEntry")
    .def(init())
    .def(init<DirectoryEntry::Type, unsigned int, std::string>())
    .def(init<const DirectoryEntry&>())
    .def_readwrite("type", &DirectoryEntry::m_type)
    .def_readwrite("id", &DirectoryEntry::m_id)
    .def_readwrite("name", &DirectoryEntry::m_name)
    .def_property_readonly_static("root_account",
      [] (const object&) { return DirectoryEntry::GetRootAccount(); })
    .def_property_readonly_static("star_directory",
      [] (const object&) { return DirectoryEntry::GetStarDirectory(); })
    .def_static("make_account", static_cast<DirectoryEntry (*)(
      unsigned int, std::string)>(&DirectoryEntry::MakeAccount))
    .def_static("make_account", static_cast<DirectoryEntry (*)(unsigned int)>(
      &DirectoryEntry::MakeAccount))
    .def_static("make_directory", static_cast<DirectoryEntry (*)(
      unsigned int, std::string)>(&DirectoryEntry::MakeDirectory))
    .def_static("make_directory", static_cast<DirectoryEntry (*)(unsigned int)>(
      &DirectoryEntry::MakeDirectory))
    .def("__hash__", static_cast<std::size_t (*)(const DirectoryEntry&)>(
      &Beam::ServiceLocator::hash_value))
    .def(self == self)
    .def(self != self)
    .def("__str__", &lexical_cast<std::string, DirectoryEntry>);
  enum_<DirectoryEntry::Type::Type>(outer, "Type")
    .value("NONE", DirectoryEntry::Type::NONE)
    .value("ACCOUNT", DirectoryEntry::Type::ACCOUNT)
    .value("DIRECTORY", DirectoryEntry::Type::DIRECTORY);
}

void Beam::Python::ExportPermissions(pybind11::module& module) {
  enum_<Permission::Type>(module, "Permission")
    .value("NONE", Permission::NONE)
    .value("READ", Permission::READ)
    .value("MOVE", Permission::MOVE)
    .value("ADMINISTRATE", Permission::ADMINISTRATE);
  ExportEnumSet<Permissions>(module, "PermissionSet");
}

void Beam::Python::ExportServiceEntry(pybind11::module& module) {
  class_<ServiceEntry>(module, "ServiceEntry")
    .def(init())
    .def(init<std::string, JsonObject, int, DirectoryEntry>())
    .def(init<const ServiceEntry&>())
    .def_property_readonly("name", &ServiceEntry::GetName)
    .def_property_readonly("properties", &ServiceEntry::GetProperties)
    .def_property_readonly("id", &ServiceEntry::GetId)
    .def_property_readonly("account", &ServiceEntry::GetAccount)
    .def(self == self)
    .def(self != self);
}

void Beam::Python::ExportServiceLocator(pybind11::module& module) {
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

void Beam::Python::ExportServiceLocatorClient(pybind11::module& module) {
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
    .def("set_credentials", &VirtualServiceLocatorClient::SetCredentials)
    .def("open", &VirtualServiceLocatorClient::Open)
    .def("close", &VirtualServiceLocatorClient::Close);
  ExportQueueSuite<AccountUpdate>(module, "AccountUpdate");
}

void Beam::Python::ExportServiceLocatorTestEnvironment(
    pybind11::module& module) {
  class_<ServiceLocatorTestEnvironment>(module, "ServiceLocatorTestEnvironment")
    .def(init())
    .def("open", &ServiceLocatorTestEnvironment::Open,
      call_guard<GilRelease>())
    .def("close", &ServiceLocatorTestEnvironment::Close,
      call_guard<GilRelease>())
    .def("get_root", &ServiceLocatorTestEnvironment::GetRoot,
      return_value_policy::reference_internal)
    .def("build_client",
      [] (ServiceLocatorTestEnvironment& self) {
        return MakeToPythonServiceLocatorClient(self.BuildClient());
      });
}
