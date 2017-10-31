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

  struct PythonToObjectConverter {
    static void* convertible(PyObject* object) {
      return object;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object f{handle};
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<
        ObjectParameter>*>(data)->storage.bytes;
      new(storage) ObjectParameter{f};
      data->convertible = storage;
    }
  };
}

void Beam::Python::ExportFunctions() {
  boost::python::to_python_converter<ObjectParameter,
    ObjectParameterToPython>();
  boost::python::converter::registry::push_back(
    &PythonToObjectConverter::convertible,
    &PythonToObjectConverter::construct,
    boost::python::type_id<int>());
  ExportFunction<std::function<void ()>>("VoidFunction");
  ExportFunction<std::function<void (const boost::python::object&)>>(
    "VoidFunctionP1");
}
