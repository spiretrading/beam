#ifndef BEAM_PYTHON_ENUM_HPP
#define BEAM_PYTHON_ENUM_HPP
#include <pybind11/pybind11.h>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {

  /**
   * Implements a type caster for Beam::Enum types.
   * @param <T> The type of enum to cast.
   */
  template<typename T>
  struct EnumTypeCaster : BasicTypeCaster<T> {
    using Type = T;
    static pybind11::handle cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
  };

  template<typename T>
  pybind11::handle EnumTypeCaster<T>::cast(Type value,
      pybind11::return_value_policy policy, pybind11::handle parent) {
    return pybind11::cast(static_cast<typename Type::Type>(value)).release();
  }

  template<typename T>
  bool EnumTypeCaster<T>::load(pybind11::handle source, bool) {
    try {
      m_value.emplace(source.cast<typename Type::Type>());
    } catch(const pybind11::cast_error&) {
      return false;
    }
    return true;
  }
}

namespace pybind11::detail {
  template<typename T, std::size_t N>
  struct type_caster<Beam::Enum<T, N>> : Beam::Python::EnumTypeCaster<
    Beam::Enum<T, N>> {};
}

#endif