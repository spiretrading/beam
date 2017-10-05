#include "Beam/Python/Exception.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Python::Details;
using namespace std;

map<type_index, PyObject*> BaseRegistry::m_exceptionRegistry;

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
