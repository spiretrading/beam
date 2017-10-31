#ifndef BEAM_PYTHONENUM_HPP
#define BEAM_PYTHONENUM_HPP
#include <boost/python.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  /*! \struct EnumClassToPythonConverter
      \brief Converts an Enum<T> to a Python object.
      \tparam T The Enum type to convert.
   */
  template<typename T>
  struct EnumClassToPythonConverter {

    //! The Enum type to convert.
    using Type = T;

    //! Converts an Enum to a Python object.
    /*!
      \param value The value to convert.
      \return The Python object represented by the <i>value</i>.
    */
    static PyObject* convert(const Type& value);
  };

  /*! \struct EnumClassFromPythonConverter
      \brief Converts an Enum from a Python object.
      \tparam T The Enum type to convert.
   */
  template<typename T>
  struct EnumClassFromPythonConverter {

    //! The Enum type to convert.
    using Type = T;

    //! Tests if a Python object can be converted to an Enum.
    /*!
      \param object The Python object to test.
      \return The <i>object</i> iff it is convertible to an Enum.
    */
    static void* convertible(PyObject* object);

    //! Constructs an Enum type from a Python object.
    /*!
      \param object The object to convert.
      \param data The storage to use for the conversion.
    */
    static void construct(PyObject* object,
      boost::python::converter::rvalue_from_python_stage1_data* data);
  };

  //! Exports an Enum.
  template<typename T>
  void ExportEnum() {
    boost::python::to_python_converter<T, EnumClassToPythonConverter<T>>();
    boost::python::converter::registry::push_back(
      &EnumClassFromPythonConverter<T>::convertible,
      &EnumClassFromPythonConverter<T>::construct,
      boost::python::type_id<T>());
  }

  template<typename T>
  PyObject* EnumClassToPythonConverter<T>::convert(const Type& value) {
    return boost::python::incref(boost::python::object{
      static_cast<typename Type::Type>(value)}.ptr());
  }

  template<typename T>
  void* EnumClassFromPythonConverter<T>::convertible(PyObject* object) {
    if(boost::python::extract<typename Type::Type>(object).check()) {
      return object;
    }
    return nullptr;
  }

  template<typename T>
  void EnumClassFromPythonConverter<T>::construct(PyObject* object,
      boost::python::converter::rvalue_from_python_stage1_data* data) {
    auto storage = reinterpret_cast<
      boost::python::converter::rvalue_from_python_storage<Type>*>(
      data)->storage.bytes;
    new(storage) Type{boost::python::extract<typename Type::Type>(object)()};
    data->convertible = storage;
  }
}
}

#endif
