#ifndef BEAM_PYTHONLISTTOVECTOR_HPP
#define BEAM_PYTHONLISTTOVECTOR_HPP
#include <vector>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "Beam/Python/BoostPython.hpp"

namespace Beam {
namespace Python {

  /*! \struct VectorFromPythonConverter
      \brief Converts a Python list to a vector.
      \tparam T The type to convert.
   */
  template<typename T>
  struct VectorFromPythonConverter {

    //! The type to convert.
    using Type = T;

    //! Tests if a Python object can be converted to a vector.
    /*!
      \param object The Python object to test.
      \return The <i>object</i> iff it is convertible to an Enum.
    */
    static void* convertible(PyObject* object);

    //! Constructs a vector from a Python object.
    /*!
      \param object The object to convert.
      \param data The storage to use for the conversion.
    */
    static void construct(PyObject* object,
      boost::python::converter::rvalue_from_python_stage1_data* data);
  };

  //! Exports a vector.
  /*!
    \param name The name of the class to export.
  */
  template<typename T>
  void ExportVector(const char* name) {
    boost::python::class_<T>(name)
      .def(boost::python::vector_indexing_suite<T>());
    boost::python::converter::registry::push_back(
      &VectorFromPythonConverter<T>::convertible,
      &VectorFromPythonConverter<T>::construct, boost::python::type_id<T>());
  }

  template<typename T>
  void* VectorFromPythonConverter<T>::convertible(PyObject* object) {
    if(PyList_Check(object)) {
      return object;
    }
    return nullptr;
  }

  template<typename T>
  void VectorFromPythonConverter<T>::construct(PyObject* object,
      boost::python::converter::rvalue_from_python_stage1_data* data) {
    boost::python::list source{boost::python::handle<>(
      boost::python::borrowed(object))};
    Type destination;
    auto length = boost::python::len(source);
    for(int i = 0; i != length; ++i) {
      destination.push_back(boost::python::extract<typename Type::value_type>(
        source[i]));
    }
    auto storage = reinterpret_cast<
      boost::python::converter::rvalue_from_python_storage<Type>*>(
      data)->storage.bytes;
    new(storage) Type(std::move(destination));
    data->convertible = storage;
  }
}
}

#endif
