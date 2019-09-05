#ifndef BEAM_PYTHON_FIXED_STRING_HPP
#define BEAM_PYTHON_FIXED_STRING_HPP
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Utilities/FixedString.hpp"

namespace Beam::Python {

  /**
   * Implements a type caster for FixedStrings.
   * @param <N> The size of the FixedString to cast.
   */
  template<typename T>
  struct FixedStringTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static pybind11::handle cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
  };

  template<typename T>
  pybind11::handle FixedStringTypeCaster<T>::cast(const Type& value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return pybind11::cast(value.GetData());
  }

  template<typename T>
  bool FixedStringTypeCaster<T>::load(pybind11::handle source, bool) {
    if(!PyUnicode_Check(source.ptr())) {
      return false;
    }
    m_value.emplace(PyUnicode_AsUTF8(object));
  }
}

#endif
