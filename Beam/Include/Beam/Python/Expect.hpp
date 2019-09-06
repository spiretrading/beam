#ifndef BEAM_PYTHON_EXPECT_HPP
#define BEAM_PYTHON_EXPECT_HPP
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Beam/Utilities/Expect.hpp"

namespace Beam::Python {

  /**
   * Exports the Expect class template.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void ExportExpect(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Expect";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Expect<T>>(module, name.c_str())
      .def(pybind11::init())
      .def(pybind11::init<const T&>())
      .def_property_readonly("is_value", &Expect<T>::IsValue)
      .def_property_readonly("is_exception", &Expect<T>::IsException)
      .def_property_readonly("value",
        static_cast<const T& (Expect<T>::*)() const>(&Expect<T>::Get));
    if(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Expect<T>, Expect<pybind11::object>>();
    }
  }
}

#endif
