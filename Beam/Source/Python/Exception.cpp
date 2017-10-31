#include "Beam/Python/Exception.hpp"
#include <stdexcept>

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Python::Details;
using namespace boost;
using namespace boost::python;
using namespace std;

map<type_index, PyObject*> BaseRegistry::m_exceptionRegistry;

namespace {
  struct RuntimeErrorToPython {
    static PyObject* convert(const runtime_error& value) {
      auto parameters = Py_BuildValue(value.what());
      auto result = PyObject_CallObject(PyExc_RuntimeError, parameters);
      return incref(result);
    }
  };
}

PyObject* BaseRegistry::GetExceptionClass(const type_index& type) {
  if(type == typeid(void)) {
    return PyExc_Exception;
  }
  auto exceptionIterator = m_exceptionRegistry.find(type);
  if(exceptionIterator == m_exceptionRegistry.end()) {
    return PyExc_Exception;
  }
  return exceptionIterator->second;
}

void BaseRegistry::SetExceptionClass(const type_index& type,
    PyObject* exceptionClass) {
  m_exceptionRegistry[type] = exceptionClass;
}

void Beam::Python::ExportExceptionPtr() {
  class_<std::exception_ptr>("ExceptionPtr", init<>());
  def("rethrow_exception", &std::rethrow_exception);
}

void Beam::Python::ExportRuntimeError() {
  class_<runtime_error>("runtime_error", init<const string&>())
    .def("__str__", &runtime_error::what);
}
