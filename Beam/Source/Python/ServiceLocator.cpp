#include "Beam/Python/ServiceLocator.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/Vector.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServiceLocator/VirtualServiceLocatorClient.hpp"
#include "Beam/ServiceLocatorTests/ServiceLocatorTestInstance.hpp"
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
  using ServiceLocatorClientSessionBuilder = ServiceProtocolClientBuilder<
    MessageProtocol<std::unique_ptr<TcpSocketChannel>,
    BinarySender<SharedBuffer>>, LiveTimer>;
  using Client = ServiceLocatorClient<ServiceLocatorClientSessionBuilder>;

  VirtualServiceLocatorClient* BuildServiceLocator(const IpAddress& address) {
    auto isConnected = false;
    ServiceLocatorClientSessionBuilder sessionBuilder(
      [=] () mutable {
        if(isConnected) {
          throw NotConnectedException();
        }
        isConnected = true;
        return std::make_unique<TcpSocketChannel>(address,
          Ref(*GetSocketThreadPool()));
      },
      [=] {
        return std::make_unique<LiveTimer>(seconds(10),
          Ref(*GetTimerThreadPool()));
      });
    auto baseClient = std::make_unique<Client>(sessionBuilder);
    auto client = new WrapperServiceLocatorClient<std::unique_ptr<Client>>(
      std::move(baseClient));
    return client;
  }
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
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportServiceLocatorTestInstance();
  }
}

void Beam::Python::ExportServiceLocatorClient() {
  class_<VirtualServiceLocatorClient, boost::noncopyable>(
    "ServiceLocatorClient", no_init)
    .def("__init__", make_constructor(&BuildServiceLocator))
    .add_property("account", &VirtualServiceLocatorClient::GetAccount)
    .add_property("session_id", &VirtualServiceLocatorClient::GetSessionId)
    .def("get_encrypted_session_id",
      &VirtualServiceLocatorClient::GetEncryptedSessionId)
    .def("authenticate_account",
      BlockingFunction(&VirtualServiceLocatorClient::AuthenticateAccount))
    .def("authenticate_session",
      BlockingFunction(&VirtualServiceLocatorClient::AuthenticateSession))
    .def("locate", BlockingFunction(&VirtualServiceLocatorClient::Locate))
    .def("register", BlockingFunction(&VirtualServiceLocatorClient::Register))
    .def("load_all_accounts",
      BlockingFunction(&VirtualServiceLocatorClient::LoadAllAccounts))
    .def("find_account",
      BlockingFunction(&VirtualServiceLocatorClient::FindAccount))
    .def("make_account",
      BlockingFunction(&VirtualServiceLocatorClient::MakeAccount))
    .def("make_directory",
      BlockingFunction(&VirtualServiceLocatorClient::MakeDirectory))
    .def("store_password",
      BlockingFunction(&VirtualServiceLocatorClient::StorePassword))
    .def("load_directory_entry", BlockingFunction(
      static_cast<DirectoryEntry (VirtualServiceLocatorClient::*)(
      const DirectoryEntry&, const std::string&)>(
      &VirtualServiceLocatorClient::LoadDirectoryEntry)))
    .def("load_directory_entry", BlockingFunction(
      static_cast<DirectoryEntry (VirtualServiceLocatorClient::*)(
      unsigned int)>(&VirtualServiceLocatorClient::LoadDirectoryEntry)))
    .def("load_parents",
      BlockingFunction(&VirtualServiceLocatorClient::LoadParents))
    .def("load_children",
      BlockingFunction(&VirtualServiceLocatorClient::LoadChildren))
    .def("delete", BlockingFunction(&VirtualServiceLocatorClient::Delete))
    .def("associate",
      BlockingFunction(&VirtualServiceLocatorClient::Associate))
    .def("detach", BlockingFunction(&VirtualServiceLocatorClient::Detach))
    .def("has_permissions",
      BlockingFunction(&VirtualServiceLocatorClient::HasPermissions))
    .def("store_permissions",
      BlockingFunction(&VirtualServiceLocatorClient::StorePermissions))
    .def("load_registration_time",
      BlockingFunction(&VirtualServiceLocatorClient::LoadRegistrationTime))
    .def("load_last_login_time",
      BlockingFunction(&VirtualServiceLocatorClient::LoadLastLoginTime))
    .def("set_credentials",
      BlockingFunction(&VirtualServiceLocatorClient::SetCredentials))
    .def("open", BlockingFunction(&VirtualServiceLocatorClient::Open))
    .def("close", BlockingFunction(&VirtualServiceLocatorClient::Close));
}

void Beam::Python::ExportServiceLocatorTestInstance() {
  class_<ServiceLocatorTestInstance, boost::noncopyable>(
      "ServiceLocatorTestInstance", init<>())
    .def("__del__", BlockingFunction(&ServiceLocatorTestInstance::Close))
    .def("open", BlockingFunction(&ServiceLocatorTestInstance::Open))
    .def("close", BlockingFunction(&ServiceLocatorTestInstance::Close))
    .def("get_root", &ServiceLocatorTestInstance::GetRoot,
      return_internal_reference<>())
    .def("build_client",
      ReleaseUniquePtr(&ServiceLocatorTestInstance::BuildClient));
}
