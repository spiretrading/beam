#include "Beam/Python/RegistryService.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include "Beam/Python/Beam.hpp"
#include "Beam/Python/ToPythonRegistryClient.hpp"
#include "Beam/RegistryServiceTests/RegistryServiceTestEnvironment.hpp"
#include "Beam/Services/ApplicationDefinitions.hpp"

using namespace Beam;
using namespace Beam::RegistryService;
using namespace Beam::RegistryService::Tests;
using namespace Beam::Services;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace pybind11;

namespace {
  auto registryClientBox = std::unique_ptr<class_<RegistryClientBox>>();
}

class_<RegistryClientBox>& Beam::Python::GetExportedRegistryClientBox() {
  return *registryClientBox;
}

void Beam::Python::ExportApplicationRegistryClient(module& module) {
  ExportRegistryClient<ToPythonRegistryClient<RegistryClient<
    DefaultSessionBuilder>>>(module, "ApplicationRegistryClient").
    def(init([] (ServiceLocatorClientBox serviceLocatorClient) {
      return std::make_shared<ToPythonRegistryClient<RegistryClient<
        DefaultSessionBuilder>>>(MakeDefaultSessionBuilder(
          std::move(serviceLocatorClient), RegistryService::SERVICE_NAME));
    }), call_guard<GilRelease>());
}

void Beam::Python::ExportRegistryEntry(module& module) {
  auto outer = class_<RegistryEntry>(module, "RegistryEntry").
    def(init()).
    def(init<RegistryEntry::Type, std::uint64_t, std::string,
      std::uint64_t>()).
    def_readwrite("type", &RegistryEntry::m_type).
    def_readwrite("id", &RegistryEntry::m_id).
    def_readwrite("name", &RegistryEntry::m_name).
    def_readwrite("version", &RegistryEntry::m_version).
    def_property_readonly_static("root", &RegistryEntry::GetRoot).
    def("__hash__", static_cast<std::size_t (*)(const RegistryEntry&)>(
      &Beam::RegistryService::hash_value)).
    def(self < self).
    def(self == self).
    def(self != self).
    def("__str__", &lexical_cast<std::string, RegistryEntry>);
  enum_<RegistryEntry::Type::Type>(outer, "Type").
    value("NONE", RegistryEntry::Type::NONE).
    value("DIRECTORY", RegistryEntry::Type::DIRECTORY).
    value("VALUE", RegistryEntry::Type::VALUE);
}

void Beam::Python::ExportRegistryService(module& module) {
  auto submodule = module.def_submodule("registry_service");
  registryClientBox = std::make_unique<class_<RegistryClientBox>>(
    ExportRegistryClient<RegistryClientBox>(submodule, "RegistryClient"));
  ExportApplicationRegistryClient(submodule);
  ExportRegistryEntry(submodule);
  auto test_module = submodule.def_submodule("tests");
  ExportRegistryServiceTestEnvironment(test_module);
}

void Beam::Python::ExportRegistryServiceTestEnvironment(module& module) {
  class_<RegistryServiceTestEnvironment>(module,
    "RegistryServiceTestEnvironment").
    def(init<ServiceLocatorClientBox>(), call_guard<GilRelease>()).
    def("__del__", [] (RegistryServiceTestEnvironment& self) {
      self.Close();
    }, call_guard<GilRelease>()).
    def("close", &RegistryServiceTestEnvironment::Close,
      call_guard<GilRelease>()).
    def("make_client", [] (RegistryServiceTestEnvironment& self,
        ServiceLocatorClientBox serviceLocatorClient) {
      return ToPythonRegistryClient(self.MakeClient(
        std::move(serviceLocatorClient)));
    }, call_guard<GilRelease>());
}
