#ifndef BEAM_PYTHON_REF_HPP
#define BEAM_PYTHON_REF_HPP
#include <boost/python.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct RefFromPythonConverter {
    static void* convertible(PyObject* object) {
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object ref{handle};
      boost::python::extract<typename T::Type*> extractor{ref};
      if(extractor.check()) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<typename T::Type>*>(data)->storage.bytes;
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object ref{handle};
      new(storage) T(Ref(*boost::python::extract<typename T::Type*>(ref)));
      data->convertible = storage;
    }
  };
}

  //! Exports a Ref.
  /*!
    \param name The name of the type.
  */
  template<typename T>
  void ExportRef(const char* name) {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::converter::registry::push_back(
      &Details::RefFromPythonConverter<T>::convertible,
      &Details::RefFromPythonConverter<T>::construct,
      boost::python::type_id<T>());
  }
}
}

#endif
