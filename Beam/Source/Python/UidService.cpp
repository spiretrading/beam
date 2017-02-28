#include "Beam/Python/UidService.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClientBuilder.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/UidService/VirtualUidClient.hpp"
#include "Beam/UidServiceTests/UidServiceTestInstance.hpp"
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

  VirtualUidClient* BuildUidClient(const IpAddress& address) {
    auto isConnected = false;
    UidClientSessionBuilder sessionBuilder(
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
    auto client = new WrapperUidClient<std::unique_ptr<Client>>(
      std::move(baseClient));
    return client;
  }
}

void Beam::Python::ExportUidService() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".uid_service");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("uid_service") = nestedModule;
  scope parent = nestedModule;
  ExportUidClient();
  {
    string nestedName = extract<string>(parent.attr("__name__") + ".tests");
    object nestedModule{handle<>(
      borrowed(PyImport_AddModule(nestedName.c_str())))};
    parent.attr("tests") = nestedModule;
    scope child = nestedModule;
    ExportUidServiceTestInstance();
  }
}

void Beam::Python::ExportUidClient() {
  class_<VirtualUidClient, boost::noncopyable>("UidClient", no_init)
    .def("__init__", make_constructor(&BuildUidClient))
    .def("load_next_uid", BlockingFunction(&VirtualUidClient::LoadNextUid))
    .def("open", BlockingFunction(&VirtualUidClient::Open))
    .def("close", BlockingFunction(&VirtualUidClient::Close));
}

void Beam::Python::ExportUidServiceTestInstance() {
  class_<UidServiceTestInstance, boost::noncopyable>("UidServiceTestInstance",
      init<>())
    .def("__del__", BlockingFunction(&UidServiceTestInstance::Close))
    .def("open", BlockingFunction(&UidServiceTestInstance::Open))
    .def("close", BlockingFunction(&UidServiceTestInstance::Close))
    .def("build_client",
      ReleaseUniquePtr(&UidServiceTestInstance::BuildClient));
}
