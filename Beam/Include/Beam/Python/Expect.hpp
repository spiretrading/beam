#ifndef BEAM_PYTHON_EXPECT_HPP
#define BEAM_PYTHON_EXPECT_HPP
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct ExpectToPython {
    static PyObject* convert(const T& expect) {
      boost::python::object value{Expect<boost::python::object>{expect}};
      boost::python::incref(value.ptr());
      return value.ptr();
    }
  };

  template<typename T>
  struct ExpectFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object expect{handle};
      boost::python::extract<Expect<boost::python::object>> extractor{expect};
      if(extractor.check()) {
        Expect<boost::python::object> value = expect;
        if(value.IsException()) {
          return object;
        }
        boost::python::extract<typename T::Type> resultExtractor{value.Get()};
        if(resultExtractor.check()) {
          return object;
        }
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<T>*>(data)->storage.bytes;
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object value{handle};
      Expect<boost::python::object> expect =
        boost::python::extract<Expect<boost::python::object>>(value);
      if(expect.IsValue()) {
        new(storage) T{boost::python::extract<typename T::Type>{expect.Get()}};
      } else {
        new(storage) T{expect.GetException()};
      }
      data->convertible = storage;
    }
  };
}

  //! Exports the Expect class for Python objects.
  void ExportExpect();

  //! Exports the Expect class template.
  /*!
    \param name The name of the class.
  */
  template<typename T>
  void ExportExpect(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Expect<T>>(name, boost::python::init<>())
      .def(boost::python::init<const T&>())
      .def("is_value", &Expect<T>::IsValue)
      .def("is_exception", &Expect<T>::IsException)
      .add_property("value", boost::python::make_function(&Expect<T>::Get,
        boost::python::return_value_policy<
        boost::python::copy_const_reference>()));
    boost::python::to_python_converter<T, Details::ExpectToPython<T>>();
    boost::python::converter::registry::push_back(
      &Details::ExpectFromPythonConverter<T>::convertible,
      &Details::ExpectFromPythonConverter<T>::construct,
      boost::python::type_id<T>());
  }
}
}

#endif
