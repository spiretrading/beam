#include "Beam/Python/UidService.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Python/ToPythonUidClient.hpp"
#include "Beam/Python/UniquePtr.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/UidServiceException.hpp"
#include "Beam/UidService/VirtualUidClient.hpp"
#include "Beam/UidServiceTests/UidServiceTestEnvironment.hpp"
#include <boost/noncopyable.hpp>

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Serialization;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  using UidClientSessionBuilder = ServiceProtocolClientBuilder<
    MessageProtocol<std::unique_ptr<TcpSocketChannel>,
    BinarySender<SharedBuffer>>, LiveTimer>;
  using Client = UidClient<UidClientSessionBuilder>;

  struct FromPythonUidClient : VirtualUidClient, wrapper<VirtualUidClient> {
    virtual std::uint64_t LoadNextUid() override final {
      return get_override("load_next_uid")();
    }

    virtual void Open() override final {
      get_override("open")();
    }

    virtual void Close() override final {
      get_override("close")();
    }
  };

  auto BuildUidClient(const IpAddress& address) {
    auto isConnected = false;
    UidClientSessionBuilder sessionBuilder{
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
    return MakeToPythonUidClient(
      std::make_unique<Client>(sessionBuilder)).release();
  }

  std::unique_ptr<VirtualUidClient> UidServiceTestEnvironmentBuildClient(
      UidServiceTestEnvironment& environment) {
    return MakeToPythonUidClient(environment.BuildClient());
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualUidClient);

void Beam::Python::ExportApplicationUidClient() {
  class_<ToPythonUidClient<Client>, bases<VirtualUidClient>,
    boost::noncopyable>("ApplicationUidClient", no_init)
    .def("__init__", make_constructor(&BuildUidClient));
}

void Beam::Python::ExportUidService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".uid_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("uid_service") = nestedModule;
  scope parent = nestedModule;
  ExportUidClient();
  ExportApplicationUidClient();
  ExportException<UidServiceException, std::runtime_error>(
    "UidServiceException")
    .def(init<const string&>());
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportUidServiceTestEnvironment();
  }
}

void Beam::Python::ExportUidClient() {
  class_<FromPythonUidClient, boost::noncopyable>("UidClient", no_init)
    .def("load_next_uid", &VirtualUidClient::LoadNextUid)
    .def("open", &VirtualUidClient::Open)
    .def("close", &VirtualUidClient::Close);
  ExportUniquePtr<VirtualUidClient>();
}

void Beam::Python::ExportUidServiceTestEnvironment() {
  class_<UidServiceTestEnvironment, boost::noncopyable>(
    "UidServiceTestEnvironment", init<>())
    .def("open", BlockingFunction(&UidServiceTestEnvironment::Open))
    .def("close", BlockingFunction(&UidServiceTestEnvironment::Close))
    .def("build_client", &UidServiceTestEnvironmentBuildClient);
}
