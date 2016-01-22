#include "Beam/Python/Yaml.hpp"
#include <fstream>
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  YAML::Node* LoadYamlFile(const string& path) {
    ifstream yamlStream{path.c_str()};
    if(!yamlStream.good()) {
      throw runtime_error{"File not found."};
    }
    YAML::Parser yamlParser{yamlStream};
    auto node = make_unique<YAML::Node>();
    yamlParser.GetNextDocument(*node);
    return node.release();
  }
}

void Beam::Python::ExportYaml() {
  class_<YAML::Node, boost::noncopyable>("YamlNode", no_init);
  def("load_yaml", LoadYamlFile, return_value_policy<manage_new_object>());
}
