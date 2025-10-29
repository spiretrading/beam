#include "Beam/Python/Utilities.hpp"
#include <fstream>
#include <pybind11/operators.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/YamlConfig.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

std::ostream& pybind11::operator <<(std::ostream& out, const object& value) {
  return out << str(value).cast<std::string>();
}

bool KeyValuePair<object, object>::operator ==(const KeyValuePair& pair) const {
  return m_key.equal(pair.m_key) && m_value.equal(pair.m_value);
}

void Beam::Python::export_key_value_pair(module& module) {
  auto key_value_pair =
    class_<KeyValuePair<object, object>>(module, "KeyValuePair").
      def(pybind11::init<object, object>()).
      def_readwrite("key", &KeyValuePair<object, object>::m_key).
      def_readwrite("value", &KeyValuePair<object, object>::m_value);
  export_default_methods(key_value_pair);
}

void Beam::Python::export_utilities(module& module) {
  export_expect<object>(module, "Expect");
  export_key_value_pair(module);
  export_yaml(module);
  module.def("is_running", &is_running);
  module.def("received_kill_event", &received_kill_event);
  module.def(
    "wait_for_kill_event", &wait_for_kill_event, call_guard<GilRelease>());
}

void Beam::Python::export_yaml(pybind11::module& module) {
  class_<YAML::Node>(module, "YamlNode");
  module.def("load_yaml", [] (std::string_view path) {
    return std::make_unique<YAML::Node>(load_file(path));
  });
}
