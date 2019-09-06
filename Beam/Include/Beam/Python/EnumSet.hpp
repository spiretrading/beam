#ifndef BEAM_PYTHON_ENUM_SET_HPP
#define BEAM_PYTHON_ENUM_SET_HPP
#include <string>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include "Beam/Collections/EnumSet.hpp"

namespace Beam::Python {

  /**
   * Exports a generic EnumSet.
   * @param module The module to export to.
   * @param name The name of the type to export.
   */
  template<typename T>
  void ExportEnumSet(pybind11::module& module, const std::string& name) {
    pybind11::class_<T>(module, name.c_str())
      .def(pybind11::init())
      .def(pybind11::init<T>())
      .def(pybind11::init<typename T::Type>())
      .def(pybind11::init<typename T::Type::Type>())
      .def("test", &T::Test)
      .def("set", &T::Set)
      .def("unset", &T::Unset)
      .def(pybind11::self == pybind11::self)
      .def(pybind11::self != pybind11::self)
      .def(pybind11::self & pybind11::self)
      .def(pybind11::self | pybind11::self)
      .def(pybind11::self ^ pybind11::self);
  }
}

#endif
