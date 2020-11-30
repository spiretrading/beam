#include "Beam/Python/UidService.hpp"
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonUidClient.hpp"
#include "Beam/UidService/ApplicationDefinitions.hpp"
#include "Beam/UidServiceTests/UidServiceTestEnvironment.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::UidService;
using namespace Beam::UidService::Tests;
using namespace pybind11;

namespace {
  auto uidClientBox = std::unique_ptr<class_<UidClientBox>>();
}

class_<UidClientBox>& Beam::Python::GetExportedUidClientBox() {
  return *uidClientBox;
}

void Beam::Python::ExportApplicationUidClient(pybind11::module& module) {
  using PythonApplicationUidClient = ToPythonUidClient<UidClient<
    DefaultSessionBuilder<ServiceLocatorClientBox>>>;
  ExportUidClient<PythonApplicationUidClient>(module, "ApplicationUidClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<PythonApplicationUidClient>(
        MakeDefaultSessionBuilder(std::move(serviceLocatorClient),
          UidService::SERVICE_NAME));
    }), call_guard<GilRelease>());
}

void Beam::Python::ExportUidService(pybind11::module& module) {
  auto submodule = module.def_submodule("uid_service");
  uidClientBox = std::make_unique<class_<UidClientBox>>(
    ExportUidClient<UidClientBox>(submodule, "UidClient"));
  ExportUidClient<ToPythonUidClient<UidClientBox>>(submodule, "UidClientBox");
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
    .def("make_client",
      [] (UidServiceTestEnvironment& self) {
        return ToPythonUidClient(self.MakeClient());
      }, call_guard<GilRelease>());
}
