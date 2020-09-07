#include "Beam/Python/UidService.hpp"
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonUidClient.hpp"
#include "Beam/UidService/ApplicationDefinitions.hpp"
#include "Beam/UidService/VirtualUidClient.hpp"
#include "Beam/UidServiceTests/UidServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  struct TrampolineUidClient final : VirtualUidClient {
    std::uint64_t LoadNextUid() override {
      PYBIND11_OVERLOAD_PURE_NAME(std::uint64_t, VirtualUidClient,
        "load_next_uid", LoadNextUid);
    }

    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualUidClient, "close", Close);
    }
  };
}

void Beam::Python::ExportApplicationUidClient(pybind11::module& module) {
  using SessionBuilder = 
    AuthenticatedServiceProtocolClientBuilder<VirtualServiceLocatorClient,
    MessageProtocol<std::unique_ptr<TcpSocketChannel>,
    BinarySender<SharedBuffer>, NullEncoder>, LiveTimer>;
  using PythonApplicationUidClient = UidClient<SessionBuilder>;
  class_<ToPythonUidClient<PythonApplicationUidClient>, VirtualUidClient>(
    module, "ApplicationUidClient")
    .def(init(
      [] (VirtualServiceLocatorClient& serviceLocatorClient) {
        auto addresses = LocateServiceAddresses(serviceLocatorClient,
          UidService::SERVICE_NAME);
        auto delay = false;
        auto sessionBuilder = SessionBuilder(
          Ref(serviceLocatorClient),
          [=] () mutable {
            if(delay) {
              auto delayTimer = LiveTimer(seconds(3),
                Ref(*GetTimerThreadPool()));
              delayTimer.Start();
              delayTimer.Wait();
            }
            delay = true;
            return std::make_unique<TcpSocketChannel>(addresses,
              Ref(*GetSocketThreadPool()));
          },
          [=] {
            return std::make_unique<LiveTimer>(seconds(10),
              Ref(*GetTimerThreadPool()));
          });
        return MakeToPythonUidClient(
          std::make_unique<PythonApplicationUidClient>(sessionBuilder));
      }), call_guard<GilRelease>());
}

void Beam::Python::ExportUidClient(pybind11::module& module) {
  class_<VirtualUidClient, TrampolineUidClient>(module, "UidClient")
    .def("load_next_uid", &VirtualUidClient::LoadNextUid)
    .def("close", &VirtualUidClient::Close);
}

void Beam::Python::ExportUidService(pybind11::module& module) {
  auto submodule = module.def_submodule("uid_service");
  ExportUidClient(submodule);
  ExportApplicationUidClient(submodule);
  auto test_module = submodule.def_submodule("tests");
  ExportUidServiceTestEnvironment(test_module);
}

void Beam::Python::ExportUidServiceTestEnvironment(pybind11::module& module) {
  class_<UidServiceTestEnvironment>(module, "UidServiceTestEnvironment")
    .def(init(), call_guard<GilRelease>())
    .def("__del__",
      [] (UidServiceTestEnvironment& self) {
        self.Close();
      }, call_guard<GilRelease>())
    .def("close", &UidServiceTestEnvironment::Close, call_guard<GilRelease>())
    .def("build_client",
      [] (UidServiceTestEnvironment& self) {
        return MakeToPythonUidClient(self.BuildClient());
      }, call_guard<GilRelease>());
}
