#ifndef BEAM_PYTHON_EXCEPTION_HPP
#define BEAM_PYTHON_EXCEPTION_HPP
#include <exception>
#include <string>
#include <tuple>
#include <pybind11/pybind11.h>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/SharedObject.hpp"

namespace Beam::Python {

  /**
   * Stores a Python exception across C++ boundaries without referencing the
   * original exception instance.
   */
  class PythonException : public std::exception {
    public:

      /**
       * Constructs a PythonException.
       * @param exception The exception instance to represent.
       */
      explicit PythonException(const pybind11::object& exception);

      /** Returns a new instance of the represented exception. */
      pybind11::object to_python() const;

      const char* what() const noexcept override;

    private:
      SharedObject m_type;
      SharedObject m_arguments;
      std::string m_message;

      PythonException(
        std::tuple<SharedObject, SharedObject, std::string> state);
  };

  inline PythonException::PythonException(const pybind11::object& exception)
    : PythonException([&] {
        auto lock = GilLock();
        return std::tuple(
          SharedObject(pybind11::reinterpret_borrow<pybind11::object>(
            reinterpret_cast<PyObject*>(Py_TYPE(exception.ptr())))),
          SharedObject(exception.attr("args")),
          std::string(pybind11::str(exception)));
      }()) {}

  inline pybind11::object PythonException::to_python() const {
    auto lock = GilLock();
    auto instance = pybind11::reinterpret_steal<pybind11::object>(
      PyObject_CallObject(m_type->ptr(), m_arguments->ptr()));
    if(instance) {
      return instance;
    }
    PyErr_Clear();
    return pybind11::reinterpret_borrow<pybind11::object>(PyExc_RuntimeError)(
      m_message);
  }

  inline const char* PythonException::what() const noexcept {
    return m_message.c_str();
  }

  inline PythonException::PythonException(
    std::tuple<SharedObject, SharedObject, std::string> state)
    : m_type(std::move(std::get<0>(state))),
      m_arguments(std::move(std::get<1>(state))),
      m_message(std::move(std::get<2>(state))) {}
}

#endif
