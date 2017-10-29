#ifndef BEAM_PYTHON_UNIQUE_PTR_HPP
#define BEAM_PYTHON_UNIQUE_PTR_HPP
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct UniquePtrToPythonConverter {
    static PyObject* convert(const T& t) {
      if(t == nullptr) {
        return Py_None;
      }
      auto object = typename boost::python::manage_new_object::apply<
        typename T::element_type*>::type{}(*t);
      const_cast<T&>(t).release();
      return object;
    }
  };
}

  //! Exports an std::unique_ptr.
  template<typename T>
  void ExportUniquePtr() {
    auto typeId = boost::python::type_id<T>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<T,
      Details::UniquePtrToPythonConverter<T>>();
  }
}
}

#endif
