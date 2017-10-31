#include "Beam/Python/Function.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;

namespace {
  struct ObjectParameterToPython {
    static PyObject* convert(const ObjectParameter& parameter) {
      auto value = parameter().ptr();
      Py_IncRef(value);
      return value;
    }
  };
}

void Beam::Python::ExportFunctions() {
  boost::python::to_python_converter<ObjectParameter,
    ObjectParameterToPython>();
  ExportFunction<std::function<void ()>>("VoidFunction");
  ExportFunction<std::function<void (const boost::python::object&)>>(
    "VoidFunctionP1");
}
