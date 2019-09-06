#ifndef BEAM_PYTHON_OUT_HPP
#define BEAM_PYTHON_OUT_HPP
#include <pybind11/pybind11.h>
#include "Beam/Pointers/Out.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {

  /**
   * Implements a type caster for Beam::Out types.
   * @param <T> The type of Out to cast.
   */
  template<typename T>
  struct OutTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
  };

  template<typename T>
  pybind11::handle OutTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return pybind11::cast(&*value);
  }

  template<typename T>
  bool OutTypeCaster<T>::load(pybind11::handle source, bool) {
    try {
      m_value.emplace(Store(*source.cast<typename Type::Type*>()));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::Out<T>> :
    Beam::Python::OutTypeCaster<Beam::Out<T>> {};
}

#endif
