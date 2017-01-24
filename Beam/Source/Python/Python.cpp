#include "Beam/Python/Python.hpp"
#include <iostream>
#include <boost/python/extract.hpp>
#include <boost/python/object.hpp>
#include <frameobject.h>
#include <sstream>

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

void Beam::Python::PrintError() {
  PyObject* type;
  PyObject* value;
  PyObject* traceback;
  PyErr_Fetch(&type, &value, &traceback);
  PyErr_NormalizeException(&type, &value, &traceback);
  auto tracebackObject = reinterpret_cast<PyTracebackObject*>(traceback);
  stringstream errorMessage;
  errorMessage << "Python Error:\n";
  if(tracebackObject != nullptr) {
    errorMessage << "  Traceback:\n";
    while(tracebackObject != nullptr) {
      auto frame = tracebackObject->tb_frame;
      auto file = PyString_AsString(frame->f_code->co_filename);
      auto line = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
      auto function = PyString_AsString(frame->f_code->co_name);
      errorMessage << "  File: " << file << "\n";
      errorMessage << "    Line: " << line << "\n";
      errorMessage << "    Function: " << function << "\n";
      tracebackObject = tracebackObject->tb_next;
    }
  }
  if(type != nullptr) {
    auto repr = PyObject_Repr(type);
    string message{PyString_AsString(repr)};
    Py_DecRef(repr);
    Py_DecRef(type);
    errorMessage << "  Type: " << message << "\n";
  }
  if(value != nullptr) {
    auto repr = PyObject_Repr(value);
    string message{PyString_AsString(repr)};
    Py_DecRef(repr);
    Py_DecRef(value);
    errorMessage << "  Message: " << message << "\n";
  }
  cerr << errorMessage.str();
}
