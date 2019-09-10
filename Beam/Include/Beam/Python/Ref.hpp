#ifndef BEAM_PYTHON_REF_HPP
#define BEAM_PYTHON_REF_HPP
#include <pybind11/pybind11.h>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {

  /**
   * Implements a type caster for Beam::Ref types.
   * @param <T> The type of Ref to cast.
   */
  template<typename T>
  struct RefTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
    using BasicTypeCaster<T>::m_value;
  };

  template<typename T>
  pybind11::handle RefTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return pybind11::cast(&*value).release();
  }

  template<typename T>
  bool RefTypeCaster<T>::load(pybind11::handle source, bool) {
    try {
      m_value.emplace(Ref(source.cast<typename Type::Type&>()));
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

namespace pybind11::detail {
  template<typename T>
  struct type_caster<Beam::Ref<T>> :
    Beam::Python::RefTypeCaster<Beam::Ref<T>> {};
}

#endif
