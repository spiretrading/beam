#include "Beam/Python/ServiceLocator.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/ToPythonServiceLocatorClient.hpp"
#include "Beam/Python/UniquePtr.hpp"
#include "Beam/Python/Vector.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/AuthenticationException.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include <boost/noncopyable.hpp>

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::ServiceLocator::Tests;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  using ApplicationServiceLocatorClientSessionBuilder =
    ServiceProtocolClientBuilder<MessageProtocol<
    std::unique_ptr<TcpSocketChannel>, BinarySender<SharedBuffer>>, LiveTimer>;
  using PythonApplicationServiceLocatorClient =
    ServiceLocatorClient<ApplicationServiceLocatorClientSessionBuilder>;

  struct FromPythonServiceLocatorClient : VirtualServiceLocatorClient,
      wrapper<VirtualServiceLocatorClient> {
    virtual DirectoryEntry GetAccount() const override final {
      return get_override("get_account")();
    }

    virtual std::string GetSessionId() const override final {
      return get_override("get_session_id")();
    }

    virtual std::string GetEncryptedSessionId(
        unsigned int key) const override final {
      return get_override("get_encrypted_session_id")(key);
    }

    virtual DirectoryEntry AuthenticateAccount(const std::string& username,
        const std::string& password) override final {
      return get_override("authenticate_account")(username, password);
    }

    virtual DirectoryEntry AuthenticateSession(const std::string& sessionId,
        unsigned int key) override final {
      return get_override("authenticate_session")(sessionId, key);
    }

    virtual std::vector<ServiceEntry> Locate(
        const std::string& name) override final{
      return get_override("locate")(name);
    }

    virtual ServiceEntry Register(const std::string& name,
        const JsonObject& properties) override final {
      return get_override("register")(name, properties);
    }

    virtual std::vector<DirectoryEntry> LoadAllAccounts() override final {
      return get_override("load_all_accounts")();
    }

    virtual boost::optional<DirectoryEntry> FindAccount(
        const std::string& name) override final {
      return get_override("find_account")(name);
    }

    virtual DirectoryEntry MakeAccount(const std::string& name,
        const std::string& password,
        const DirectoryEntry& parent) override final {
      return get_override("make_account")(name, password, parent);
    }

    virtual DirectoryEntry MakeDirectory(const std::string& name,
        const DirectoryEntry& parent) {
      return get_override("make_directory")(name, parent);
    }

    virtual void StorePassword(const DirectoryEntry& account,
        const std::string& password) override final {
      get_override("store_password")(account, password);
    }

    virtual DirectoryEntry LoadDirectoryEntry(const DirectoryEntry& root,
        const std::string& path) override final {
      return get_override("load_directory_entry")(root, path);
    }

    virtual DirectoryEntry LoadDirectoryEntry(unsigned int id) override final {
      return get_override("load_directory_entry")(id);
    }

    virtual std::vector<DirectoryEntry> LoadParents(
        const DirectoryEntry& entry) override final {
      return get_override("load_parents")(entry);
    }

    virtual std::vector<DirectoryEntry> LoadChildren(
        const DirectoryEntry& entry) override final {
      return get_override("load_children")(entry);
    }

    virtual void Delete(const DirectoryEntry& entry) override final {
      get_override("delete")(entry);
    }

    virtual void Associate(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override final {
      get_override("associate")(entry, parent);
    }

    virtual void Detach(const DirectoryEntry& entry,
        const DirectoryEntry& parent) override final {
      get_override("detach")(entry, parent);
    }

    virtual bool HasPermissions(const DirectoryEntry& account,
        const DirectoryEntry& target, Permissions permissions) override final {
      return get_override("has_permissions")(account, target, permissions);
    }

    virtual void StorePermissions(const DirectoryEntry& source,
        const DirectoryEntry& target, Permissions permissions) override final {
      get_override("store_permissions")(source, target, permissions);
    }

    virtual boost::posix_time::ptime LoadRegistrationTime(
        const DirectoryEntry& account) override final {
      return get_override("load_registration_time")(account);
    }

    virtual boost::posix_time::ptime LoadLastLoginTime(
        const DirectoryEntry& account) override final {
      return get_override("load_last_login_time")(account);
    }

    virtual DirectoryEntry Rename(const DirectoryEntry& entry,
        const std::string& name) override final {
      return get_override("rename")(entry, name);
    }

    virtual void SetCredentials(const std::string& username,
        const std::string& password) override final {
      get_override("set_credentials")(username, password);
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  auto MakeApplicationServiceLocatorClient(const IpAddress& address) {
    auto isConnected = false;
    ApplicationServiceLocatorClientSessionBuilder sessionBuilder{
      [=] () mutable {
        if(isConnected) {
          throw NotConnectedException{};
        }
        isConnected = true;
        return std::make_unique<TcpSocketChannel>(address,
          Ref(*GetSocketThreadPool()));
      },
      [=] {
        return std::make_unique<LiveTimer>(seconds(10),
          Ref(*GetTimerThreadPool()));
      }};
    return MakeToPythonServiceLocatorClient(
      std::make_unique<PythonApplicationServiceLocatorClient>(
      sessionBuilder)).release();
  }

  std::unique_ptr<VirtualServiceLocatorClient>
      ServiceLocatorTestEnvironmentBuildClient(
      ServiceLocatorTestEnvironment& environment) {
    return MakeToPythonServiceLocatorClient(environment.BuildClient());
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualServiceLocatorClient);

void Beam::Python::ExportApplicationServiceLocatorClient() {
  class_<ToPythonServiceLocatorClient<PythonApplicationServiceLocatorClient>,
    bases<VirtualServiceLocatorClient>, boost::noncopyable>(
    "ApplicationServiceLocatorClient", no_init)
    .def("__init__", make_constructor(&MakeApplicationServiceLocatorClient));
}

void Beam::Python::ExportDirectoryEntry() {
  {
    scope outer =
      class_<DirectoryEntry>("DirectoryEntry", init<>())
        .def(init<DirectoryEntry::Type, unsigned int, std::string>())
        .def("__copy__", &MakeCopy<DirectoryEntry>)
        .def("__deepcopy__", &MakeDeepCopy<DirectoryEntry>)
        .add_property("type", make_getter(&DirectoryEntry::m_type,
          return_value_policy<return_by_value>()), make_setter(
          &DirectoryEntry::m_type, return_value_policy<return_by_value>()))
        .def_readwrite("id", &DirectoryEntry::m_id)
        .def_readwrite("name", &DirectoryEntry::m_name)
        .add_static_property("root_account",
          &DirectoryEntry::GetRootAccount)
        .add_static_property("star_directory",
          &DirectoryEntry::GetStarDirectory)
        .def("make_account", static_cast<DirectoryEntry (*)(
          unsigned int, string)>(&DirectoryEntry::MakeAccount))
        .def("make_account", static_cast<DirectoryEntry (*)(unsigned int)>(
          &DirectoryEntry::MakeAccount))
        .staticmethod("make_account")
        .def("make_directory", static_cast<DirectoryEntry (*)(
          unsigned int, string)>(&DirectoryEntry::MakeDirectory))
        .def("make_directory", static_cast<DirectoryEntry (*)(unsigned int)>(
          &DirectoryEntry::MakeDirectory))
        .staticmethod("make_directory")
        .def("__hash__", static_cast<std::size_t (*)(const DirectoryEntry&)>(
          &Beam::ServiceLocator::hash_value))
        .def(self == self)
        .def(self != self);
    enum_<DirectoryEntry::Type::Type>("Type")
      .value("NONE", DirectoryEntry::Type::NONE)
      .value("ACCOUNT", DirectoryEntry::Type::ACCOUNT)
      .value("DIRECTORY", DirectoryEntry::Type::DIRECTORY);
  }
  ExportEnum<DirectoryEntry::Type>();
  python_optional<DirectoryEntry>();
  ExportVector<vector<DirectoryEntry>>("VectorDirectoryEntry");
}

void Beam::Python::ExportPermissions() {
  enum_<Permission::Type>("Permission")
    .value("NONE", Permission::NONE)
    .value("READ", Permission::READ)
    .value("MOVE", Permission::MOVE)
    .value("ADMINISTRATE", Permission::ADMINISTRATE);
  ExportEnum<Permission>();
  ExportEnumSet<Permission>("PermissionSet");
}

void Beam::Python::ExportServiceEntry() {
  class_<ServiceEntry>("ServiceEntry", init<>())
    .def(init<const string&, const JsonObject&, int, const DirectoryEntry&>())
    .def("__copy__", &MakeCopy<ServiceEntry>)
    .def("__deepcopy__", &MakeDeepCopy<ServiceEntry>)
    .add_property("name", make_function(&ServiceEntry::GetName,
      return_value_policy<copy_const_reference>()))
    .add_property("properties", make_function(&ServiceEntry::GetProperties,
      return_value_policy<copy_const_reference>()))
    .add_property("id", &ServiceEntry::GetId)
    .add_property("account", make_function(&ServiceEntry::GetAccount,
      return_value_policy<copy_const_reference>()))
    .def(self == self)
    .def(self != self);
  ExportVector<vector<ServiceEntry>>("VectorServiceEntry");
}

void Beam::Python::ExportServiceLocator() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".service_locator");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("service_locator") = nestedModule;
  scope parent = nestedModule;
  ExportPermissions();
  ExportDirectoryEntry();
  ExportServiceEntry();
  ExportServiceLocatorClient();
  ExportApplicationServiceLocatorClient();
  ExportException<AuthenticationException, ConnectException>(
    "AuthenticationException")
    .def(init<>())
    .def(init<const string&>());
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportServiceLocatorTestEnvironment();
  }
}

void Beam::Python::ExportServiceLocatorClient() {
  class_<FromPythonServiceLocatorClient, boost::noncopyable>(
    "ServiceLocatorClient", no_init)
    .def("get_account", pure_virtual(&VirtualServiceLocatorClient::GetAccount))
    .def("get_session_id",
      pure_virtual(&VirtualServiceLocatorClient::GetSessionId))
    .def("get_encrypted_session_id",
      pure_virtual(&VirtualServiceLocatorClient::GetEncryptedSessionId))
    .def("authenticate_account",
      pure_virtual(&VirtualServiceLocatorClient::AuthenticateAccount))
    .def("authenticate_session",
      pure_virtual(&VirtualServiceLocatorClient::AuthenticateSession))
    .def("locate", pure_virtual(&VirtualServiceLocatorClient::Locate))
    .def("register", pure_virtual(&VirtualServiceLocatorClient::Register))
    .def("load_all_accounts",
      pure_virtual(&VirtualServiceLocatorClient::LoadAllAccounts))
    .def("find_account",
      pure_virtual(&VirtualServiceLocatorClient::FindAccount))
    .def("make_account",
      pure_virtual(&VirtualServiceLocatorClient::MakeAccount))
    .def("make_directory",
      pure_virtual(&VirtualServiceLocatorClient::MakeDirectory))
    .def("store_password",
      pure_virtual(&VirtualServiceLocatorClient::StorePassword))
    .def("load_directory_entry", pure_virtual(
      static_cast<DirectoryEntry (VirtualServiceLocatorClient::*)(
      const DirectoryEntry&, const std::string&)>(
      &VirtualServiceLocatorClient::LoadDirectoryEntry)))
    .def("load_directory_entry", pure_virtual(static_cast<
      DirectoryEntry (VirtualServiceLocatorClient::*)(unsigned int)>(
      &VirtualServiceLocatorClient::LoadDirectoryEntry)))
    .def("load_parents",
      pure_virtual(&VirtualServiceLocatorClient::LoadParents))
    .def("load_children",
      pure_virtual(&VirtualServiceLocatorClient::LoadChildren))
    .def("delete", pure_virtual(&VirtualServiceLocatorClient::Delete))
    .def("associate", pure_virtual(&VirtualServiceLocatorClient::Associate))
    .def("detach", pure_virtual(&VirtualServiceLocatorClient::Detach))
    .def("has_permissions",
      pure_virtual(&VirtualServiceLocatorClient::HasPermissions))
    .def("store_permissions",
      pure_virtual(&VirtualServiceLocatorClient::StorePermissions))
    .def("load_registration_time",
      pure_virtual(&VirtualServiceLocatorClient::LoadRegistrationTime))
    .def("load_last_login_time",
      pure_virtual(&VirtualServiceLocatorClient::LoadLastLoginTime))
    .def("rename", pure_virtual(&VirtualServiceLocatorClient::Rename))
    .def("set_credentials",
      pure_virtual(&VirtualServiceLocatorClient::SetCredentials))
    .def("open", pure_virtual(&VirtualServiceLocatorClient::Open))
    .def("close", pure_virtual(&VirtualServiceLocatorClient::Close));
  ExportUniquePtr<VirtualServiceLocatorClient>();
}

void Beam::Python::ExportServiceLocatorTestEnvironment() {
  class_<ServiceLocatorTestEnvironment, boost::noncopyable>(
      "ServiceLocatorTestEnvironment", init<>())
    .def("open", BlockingFunction(&ServiceLocatorTestEnvironment::Open))
    .def("close", BlockingFunction(&ServiceLocatorTestEnvironment::Close))
    .def("get_root", &ServiceLocatorTestEnvironment::GetRoot,
      return_internal_reference<>())
    .def("build_client", &ServiceLocatorTestEnvironmentBuildClient);
}
