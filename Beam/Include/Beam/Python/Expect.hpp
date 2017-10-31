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
    static PyObject* convert(const Expect<T>& expect) {
      return MakeManagedPointer(new Expect<boost::python::object>{expect});
    }
  };

  template<typename T>
  struct ExpectFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::extract<const Expect<boost::python::object>&> extractor{
        object};
      if(extractor.check()) {
        auto& expect = extractor();
        if(expect.IsException()) {
          return object;
        }
        if(boost::python::extract<T>{expect.Get()}.check()) {
          return object;
        }
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<
        boost::python::converter::rvalue_from_python_storage<T>*>(
        data)->storage.bytes;
      auto& expect = boost::python::extract<
        const Expect<boost::python::object>&>{object}();
      if(expect.IsValue()) {
        new(storage) Expect<T>{boost::python::extract<T>{expect.Get()}()};
      } else {
        new(storage) Expect<T>{expect.GetException()};
      }
      data->convertible = storage;
    }
  };
}

  //! Exports the Expect class template.
  /*!
    \param name The name of the class.
  */
  template<typename T>
  void ExportExpect(const char* name) {
    auto typeId = boost::python::type_id<Expect<T>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::class_<Expect<T>>(name, boost::python::init<>())
      .def(boost::python::init<const T&>())
      .add_property("is_value", &Expect<T>::IsValue)
      .add_property("is_exception", &Expect<T>::IsException)
      .add_property("value", boost::python::make_function(&Expect<T>::Get,
        boost::python::return_value_policy<
        boost::python::copy_const_reference>()));
    if(!std::is_same<T, boost::python::object>::value) {
      boost::python::to_python_converter<Expect<T>,
        Details::ExpectToPython<T>>();
      boost::python::converter::registry::push_back(
        &Details::ExpectFromPythonConverter<T>::convertible,
        &Details::ExpectFromPythonConverter<T>::construct,
        boost::python::type_id<Expect<T>>());
    }
  }
}
}

#endif
