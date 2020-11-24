#include "Beam/Python/Json.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Python/Variant.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

void Beam::Python::ExportJson(module& module) {
  ExportJsonObject(module);
}

void Beam::Python::ExportJsonObject(module& module) {
  class_<JsonObject>(module, "JsonObject")
    .def(init())
    .def("__getitem__",
      [] (JsonObject& self, const std::string& key) -> decltype(auto) {
        return static_cast<Details::JsonVariant&>(self[key]);
      }, return_value_policy::reference_internal)
    .def("__setitem__",
      [] (JsonObject& self, const std::string& key, int value) {
        self[key] = value;
      })
    .def("__setitem__",
      [] (JsonObject& self, const std::string& key,
          const Details::JsonVariant& value) {
        self[key] = value;
      })
    .def("__str__", &lexical_cast<std::string, JsonObject>)
    .def(self == self)
    .def(self != self);
}
