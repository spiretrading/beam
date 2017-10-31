#ifndef BEAM_PYTHON_EXCEPTION_HPP
#define BEAM_PYTHON_EXCEPTION_HPP
#include <map>
#include <string>
#include <typeindex>
#include <boost/python.hpp>
#include "Beam/Python/PythonBindings.hpp"

namespace Beam {
namespace Python {
namespace Details {
  class BaseRegistry {
    public:
      static PyObject* GetExceptionClass(const std::type_index& type);

      static void SetExceptionClass(const std::type_index& type,
        PyObject* exceptionClass);

    private:
      static std::map<std::type_index, PyObject*> m_exceptionRegistry;
  };
}

  /*! \class PythonException
      \brief Provides functions to wrap and translate C++ exceptions to Python.
      \tparam T The type of exception to wrap.
   */
  template<typename T>
  class PythonException {
    public:

      //! Returns the Python object representing the exception type.
      static PyObject* GetExceptionClass();

      //! Constructs a Python exception type.
      template<typename Base>
      static PyObject* CreateExceptionClass(const char* name);

      //! Throws an exception.
      static void Throw(const T& exception);

    private:
      static PyObject* m_exceptionType;
  };

  //! Exports the std::exception_ptr class.
  void ExportExceptionPtr();

  //! Exports the std::runtime_error class.
  void ExportRuntimeError();

  template<typename T, typename Base>
  auto ExportException(const char* name) {
    auto c = boost::python::class_<T, boost::python::bases<Base>>(name,
      boost::python::no_init)
      .def("__str__", &T::what);
    PythonException<T>::template CreateExceptionClass<Base>(name);
    return c;
  }

  template<typename T>
  PyObject* PythonException<T>::GetExceptionClass() {
    return m_exceptionType;
  }

  template<typename T>
  template<typename Base>
  PyObject* PythonException<T>::CreateExceptionClass(const char* name) {
    if(m_exceptionType != nullptr) {
      return m_exceptionType;
    }
    std::string scope = boost::python::extract<std::string>(
      boost::python::scope().attr("__name__"));
    auto qualifiedName = scope + "." + name;
    auto exceptionType = PyErr_NewException(
      const_cast<char*>(qualifiedName.c_str()),
      Details::BaseRegistry::GetExceptionClass(typeid(Base)), nullptr);
    if(exceptionType == nullptr) {
      boost::python::throw_error_already_set();
    }
    boost::python::scope().attr(name) =
      boost::python::handle<>(boost::python::borrowed(exceptionType));
    m_exceptionType = exceptionType;
    boost::python::register_exception_translator<T>(&Throw);
    Details::BaseRegistry::SetExceptionClass(typeid(T), m_exceptionType);
    return m_exceptionType;
  }

  template<typename T>
  void PythonException<T>::Throw(const T& exception) {
    boost::python::object pythonException{exception};
    boost::python::object translatedException{
      boost::python::handle<>(boost::python::borrowed(m_exceptionType))};
    translatedException.attr("cause") = pythonException;
    PyErr_SetString(m_exceptionType, exception.what());
  }

  template<typename T>
  PyObject* PythonException<T>::m_exceptionType;
}
}

#endif
