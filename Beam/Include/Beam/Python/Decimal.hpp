#ifndef BEAM_PYTHONDECIMAL_HPP
#define BEAM_PYTHONDECIMAL_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  inline auto PyDecimal() {
    static boost::python::object decimal =
      boost::python::import("decimal").attr("Decimal");
    return decimal;
  }

  template<typename T>
  struct DecimalToPython {
    static PyObject* convert(const T& value) {
      auto result = PyDecimal()(static_cast<std::string>(value));
      return boost::python::incref(result.ptr());
    }
  };

  template<typename T>
  struct DecimalFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(PyObject_IsInstance(object, PyDecimal().ptr())) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<T>*>(
        data)->storage.bytes;
      auto str = PyObject_Str(object);
      auto value = PyString_AsString(str);
      new(storage) T{value};
      data->convertible = storage;
      Py_DECREF(str);
    }
  };
}

  //! Exports a Decimal.
  template<typename T>
  void ExportDecimal() {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<T, Details::DecimalToPython<T>>();
    boost::python::converter::registry::push_back(
      &Details::DecimalFromPythonConverter<T>::convertible,
      &Details::DecimalFromPythonConverter<T>::construct,
      boost::python::type_id<T>());
  }
}
}

#endif
