#ifndef BEAM_PYTHONFIXEDSTRING_HPP
#define BEAM_PYTHONFIXEDSTRING_HPP
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<std::size_t N>
  struct FixedStringToPython {
    static PyObject* convert(const FixedString<N>& value) {
      auto result = boost::python::object{value.GetData()};
      return boost::python::incref(result.ptr());
    }
  };

  template<std::size_t N>
  struct FixedStringFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(PyString_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<FixedString<N>>*>(data)->storage.bytes;
      auto value = PyUnicode_AsUTF8(object);
      new(storage) FixedString<N>{value};
      data->convertible = storage; 
    }
  };
}

  //! Exports a FixedString to python.
  template<std::size_t N>
  void ExportFixedString() {
    auto typeId = boost::python::type_id<FixedString<N>>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::to_python_converter<FixedString<N>,
      Details::FixedStringToPython<N>>();
    boost::python::converter::registry::push_back(
      &Details::FixedStringFromPythonConverter<N>::convertible,
      &Details::FixedStringFromPythonConverter<N>::construct,
      boost::python::type_id<FixedString<N>>());
  }
}
}

#endif
