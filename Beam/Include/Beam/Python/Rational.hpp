#ifndef BEAM_PYTHONRATIONAL_HPP
#define BEAM_PYTHONRATIONAL_HPP
#include <boost/python.hpp>
#include <boost/rational.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  inline auto PyFraction() {
    static boost::python::object fraction =
      boost::python::import("fractions").attr("Fraction");
    return fraction;
  }

  template<typename T>
  struct RationalToPython {
    static PyObject* convert(const T& value) {
      auto result = PyFraction()(value.numerator(), value.denominator());
      return boost::python::incref(result.ptr());
    }
  };

  template<typename T>
  struct RationalFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(PyObject_IsInstance(object, PyFraction().ptr())) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<T>*>(data)->storage.bytes;
      boost::python::handle<> handle{object};
      boost::python::object fraction{handle};
      new(storage) T{static_cast<typename T::int_type>(
        boost::python::extract<typename T::int_type>(
        fraction.attr("numerator"))), static_cast<typename T::int_type>(
        boost::python::extract<typename T::int_type>(
        fraction.attr("denominator")))};
      data->convertible = storage;
    }
  };
}

  //! Exports a rational.
  template<typename T>
  void ExportRational() {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<T, Details::RationalToPython<T>>();
    boost::python::converter::registry::push_back(
      &Details::RationalFromPythonConverter<T>::convertible,
      &Details::RationalFromPythonConverter<T>::construct,
      boost::python::type_id<T>());
  }
}
}

#endif
