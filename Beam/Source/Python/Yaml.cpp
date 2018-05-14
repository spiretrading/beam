#include "Beam/Python/Yaml.hpp"
#include <fstream>
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;

namespace {
  YAML::Node* LoadYamlFile(const std::string& path) {
    auto node = std::make_unique<YAML::Node>(LoadFile(path));
    return node.release();
  }
}

void Beam::Python::ExportYaml() {
  class_<YAML::Node, boost::noncopyable>("YamlNode", no_init);
  def("load_yaml", LoadYamlFile, return_value_policy<manage_new_object>());
}
