#include "Beam/Python/Yaml.hpp"
#include <fstream>
#include "Beam/Utilities/YamlConfig.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

namespace {
  auto LoadYamlFile(const std::string& path) {
    auto node = std::make_unique<YAML::Node>(LoadFile(path));
    return node.release();
  }
}

void Beam::Python::ExportYaml(pybind11::module& module) {
  class_<YAML::Node>(module, "YamlNode");
  module.def("load_yaml",
    [] (const std::string& path) {
      return new YAML::Node(LoadFile(path));
    }, return_value_policy::take_ownership);
}
