#ifndef BEAM_PYTHON_UNIQUE_PTR_HPP
#define BEAM_PYTHON_UNIQUE_PTR_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct UniquePtrToPythonConverter {
    static PyObject* convert(const std::unique_ptr<T>& t) {
      if(t == nullptr) {
        return Py_None;
      }
      auto object = MakeManagedPointer(t.get());
      const_cast<std::unique_ptr<T>&>(t).release();
      return object;
    }
  };
}

  //! Exports an std::unique_ptr.
  template<typename T>
  void ExportUniquePtr() {
    auto typeId = boost::python::type_id<std::unique_ptr<T>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<std::unique_ptr<T>,
      Details::UniquePtrToPythonConverter<T>>();
  }
}
}

#endif
